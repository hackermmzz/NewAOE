#include "Server/render/game/engine/TextureCache.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <system_error>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

namespace newaoe::render::detail {
namespace {

bool valid(sg_image value) noexcept { return value.id != 0; }
bool valid(sg_view value) noexcept { return value.id != 0; }
bool valid(sg_sampler value) noexcept { return value.id != 0; }

std::string lookupKey(std::string path) {
#if defined(_WIN32)
    std::transform(path.begin(), path.end(), path.begin(),
                   [](unsigned char character) {
                       return character >= 'A' && character <= 'Z'
                                  ? static_cast<char>(character - 'A' + 'a')
                                  : static_cast<char>(character);
                   });
#endif
    return path;
}

}  // namespace

bool TextureCache::initialize(std::string& error) {
    if (valid(sampler_)) return true;

    sg_sampler_desc samplerDescriptor{};
    samplerDescriptor.min_filter = SG_FILTER_LINEAR;
    samplerDescriptor.mag_filter = SG_FILTER_LINEAR;
    samplerDescriptor.mipmap_filter = SG_FILTER_LINEAR;
    samplerDescriptor.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    samplerDescriptor.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    samplerDescriptor.label = "NewAOE texture sampler";
    sampler_ = sg_make_sampler(&samplerDescriptor);
    if (sg_query_sampler_state(sampler_) != SG_RESOURCESTATE_VALID) {
        error = "failed to create the shared texture sampler";
        shutdown();
        return false;
    }

    if (!createSolidTexture(0xFFFFFFFFU, white_, "NewAOE white texture",
                            error) ||
        !createSolidTexture(0xFFFF8080U, flatNormal_,
                            "NewAOE flat normal texture", error) ||
        !createSolidTexture(0xFF000000U, black_, "NewAOE black texture",
                            error)) {
        shutdown();
        return false;
    }
    return true;
}

void TextureCache::shutdown() noexcept {
    for (Slot& slot : slots_) {
        if (slot.occupied) destroy(slot.gpu);
        slot = {};
    }
    slots_.clear();
    freeSlots_.clear();
    byPath_.clear();
    destroy(white_);
    destroy(flatNormal_);
    destroy(black_);
    if (valid(sampler_)) sg_destroy_sampler(sampler_);
    sampler_ = {};
}

TextureHandle TextureCache::load(std::string_view path, std::string& error,
                                 bool srgb) {
    error.clear();
    if (path.empty()) {
        error = "image path is empty";
        return {};
    }

    const std::string canonicalPath = canonicalize(path);
    std::string cacheKey = lookupKey(canonicalPath);
    cacheKey += srgb ? "|srgb" : "|linear";
    const auto cached = byPath_.find(cacheKey);
    if (cached != byPath_.end()) {
        Slot& slot = slots_[cached->second];
        if (slot.occupied && slot.references !=
                                 std::numeric_limits<std::uint32_t>::max()) {
            ++slot.references;
        }
        return TextureHandle{cached->second, slot.generation};
    }

    const std::filesystem::path imagePath =
        std::filesystem::u8path(canonicalPath);
    std::ifstream stream(imagePath, std::ios::binary | std::ios::ate);
    if (!stream) {
        error = "failed to open image '" + canonicalPath + "'";
        return {};
    }
    const std::streamoff encodedByteCount = stream.tellg();
    if (encodedByteCount <= 0 ||
        encodedByteCount >
            static_cast<std::streamoff>(std::numeric_limits<int>::max())) {
        error = "image is empty or too large for stb_image: '" +
                canonicalPath + "'";
        return {};
    }
    std::vector<stbi_uc> encoded(
        static_cast<std::size_t>(encodedByteCount));
    stream.seekg(0, std::ios::beg);
    if (!stream.read(reinterpret_cast<char*>(encoded.data()),
                     static_cast<std::streamsize>(encoded.size()))) {
        error = "failed to read image '" + canonicalPath + "'";
        return {};
    }

    int width = 0;
    int height = 0;
    int sourceChannels = 0;
    stbi_uc* pixels = stbi_load_from_memory(
        encoded.data(), static_cast<int>(encoded.size()), &width, &height,
        &sourceChannels, STBI_rgb_alpha);
    if (pixels == nullptr || width <= 0 || height <= 0) {
        error = "failed to decode image '" + canonicalPath + "': ";
        error += stbi_failure_reason() != nullptr ? stbi_failure_reason()
                                                  : "unknown stb_image error";
        if (pixels != nullptr) stbi_image_free(pixels);
        return {};
    }

    const std::size_t decodedWidth = static_cast<std::size_t>(width);
    const std::size_t decodedHeight = static_cast<std::size_t>(height);
    if (decodedWidth > std::numeric_limits<std::size_t>::max() /
                           decodedHeight) {
        stbi_image_free(pixels);
        error = "decoded image dimensions overflow the upload size: '" +
                canonicalPath + "'";
        return {};
    }
    const std::size_t pixelCount = decodedWidth * decodedHeight;
    if (pixelCount > std::numeric_limits<std::size_t>::max() / 4U) {
        stbi_image_free(pixels);
        error = "decoded image dimensions overflow the upload size: '" +
                canonicalPath + "'";
        return {};
    }
    const std::size_t byteCount = pixelCount * 4U;
    sg_image_desc imageDescriptor{};
    imageDescriptor.width = width;
    imageDescriptor.height = height;
    const sg_pixelformat_info srgbInfo =
        sg_query_pixelformat(SG_PIXELFORMAT_SRGB8A8);
    imageDescriptor.pixel_format =
        srgb && srgbInfo.sample && srgbInfo.filter
            ? SG_PIXELFORMAT_SRGB8A8
            : SG_PIXELFORMAT_RGBA8;
    imageDescriptor.data.mip_levels[0] = {pixels, byteCount};
    imageDescriptor.label = "NewAOE loaded image";
    const sg_image image = sg_make_image(&imageDescriptor);
    stbi_image_free(pixels);

    if (sg_query_image_state(image) != SG_RESOURCESTATE_VALID) {
        if (valid(image)) sg_destroy_image(image);
        error = "Sokol failed to create GPU image for '" + canonicalPath + "'";
        return {};
    }

    sg_view_desc viewDescriptor{};
    viewDescriptor.texture.image = image;
    viewDescriptor.label = "NewAOE loaded image view";
    const sg_view view = sg_make_view(&viewDescriptor);
    if (sg_query_view_state(view) != SG_RESOURCESTATE_VALID) {
        if (valid(view)) sg_destroy_view(view);
        sg_destroy_image(image);
        error = "Sokol failed to create texture view for '" + canonicalPath +
                "'";
        return {};
    }

    std::uint32_t index = 0;
    if (freeSlots_.empty()) {
        index = static_cast<std::uint32_t>(slots_.size());
        slots_.emplace_back();
    } else {
        index = freeSlots_.back();
        freeSlots_.pop_back();
    }

    Slot& slot = slots_[index];
    // A zero generation is reserved for invalid handles.
    if (slot.generation == 0) slot.generation = 1;
    slot.gpu = TextureGpuResource{image, view, width, height};
    slot.canonicalPath = canonicalPath;
    slot.cacheKey = cacheKey;
    slot.references = 1;
    slot.occupied = true;
    byPath_.emplace(slot.cacheKey, index);
    return TextureHandle{index, slot.generation};
}

bool TextureCache::release(TextureHandle handle) noexcept {
    if (!handle.valid() || handle.index >= slots_.size()) return false;
    Slot& slot = slots_[handle.index];
    if (!slot.occupied || slot.generation != handle.generation) return false;
    if (slot.references > 1) {
        --slot.references;
        return true;
    }

    byPath_.erase(slot.cacheKey);
    destroy(slot.gpu);
    slot.canonicalPath.clear();
    slot.cacheKey.clear();
    slot.references = 0;
    slot.occupied = false;
    ++slot.generation;
    if (slot.generation == 0) ++slot.generation;
    freeSlots_.push_back(handle.index);
    return true;
}

const TextureGpuResource* TextureCache::resolve(
    TextureHandle handle) const noexcept {
    if (!handle.valid() || handle.index >= slots_.size()) return nullptr;
    const Slot& slot = slots_[handle.index];
    if (!slot.occupied || slot.generation != handle.generation) return nullptr;
    return &slot.gpu;
}

bool TextureCache::createSolidTexture(std::uint32_t rgba,
                                      TextureGpuResource& output,
                                      const char* label,
                                      std::string& error) {
    sg_image_desc descriptor{};
    descriptor.width = 1;
    descriptor.height = 1;
    descriptor.pixel_format = SG_PIXELFORMAT_RGBA8;
    descriptor.data.mip_levels[0] = {&rgba, sizeof(rgba)};
    descriptor.label = label;
    output.image = sg_make_image(&descriptor);
    if (sg_query_image_state(output.image) != SG_RESOURCESTATE_VALID) {
        error = std::string("failed to create ") + label;
        destroy(output);
        return false;
    }

    sg_view_desc viewDescriptor{};
    viewDescriptor.texture.image = output.image;
    viewDescriptor.label = label;
    output.view = sg_make_view(&viewDescriptor);
    if (sg_query_view_state(output.view) != SG_RESOURCESTATE_VALID) {
        error = std::string("failed to create view for ") + label;
        destroy(output);
        return false;
    }
    output.width = 1;
    output.height = 1;
    return true;
}

std::string TextureCache::canonicalize(std::string_view path) {
    std::filesystem::path value =
        std::filesystem::u8path(std::string(path));
    std::error_code error;
    value = std::filesystem::absolute(value, error);
    if (error) {
        error.clear();
        value = std::filesystem::u8path(std::string(path));
    }
    const std::filesystem::path canonical =
        std::filesystem::weakly_canonical(value, error);
    if (!error) value = canonical;
    value = value.lexically_normal();
    return value.generic_u8string();
}

void TextureCache::destroy(TextureGpuResource& resource) noexcept {
    if (valid(resource.view)) sg_destroy_view(resource.view);
    if (valid(resource.image)) sg_destroy_image(resource.image);
    resource = {};
}

}  // namespace newaoe::render::detail
