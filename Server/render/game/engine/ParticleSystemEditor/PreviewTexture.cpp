#include "Server/render/game/engine/ParticleSystemEditor/PreviewTexture.h"

#include <fstream>
#include <limits>
#include <utility>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#include "sokol_app.h"
#include "util/sokol_imgui.h"

namespace newaoe::render::editor {

PreviewTexture::~PreviewTexture() {
    reset();
}

bool PreviewTexture::load(const std::filesystem::path& path,
                          std::string& error) {
    const std::string displayPath = path.generic_u8string();
    int width = 0;
    int height = 0;
    int channels = 0;
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) {
        error = "Could not open texture '" + displayPath + "'";
        return false;
    }
    const std::streamoff byteCount = stream.tellg();
    if (byteCount <= 0 ||
        byteCount > static_cast<std::streamoff>(
                        std::numeric_limits<int>::max())) {
        error = "Texture is empty or too large for stb_image: '" +
                displayPath + "'";
        return false;
    }
    std::vector<stbi_uc> encoded(static_cast<std::size_t>(byteCount));
    stream.seekg(0, std::ios::beg);
    if (!stream.read(reinterpret_cast<char*>(encoded.data()), byteCount)) {
        error = "Could not read texture '" + displayPath + "'";
        return false;
    }
    stbi_uc* pixels = stbi_load_from_memory(
        encoded.data(), static_cast<int>(encoded.size()), &width, &height,
        &channels, 4);
    if (!pixels) {
        const char* reason = stbi_failure_reason();
        error = "Could not load texture '" + displayPath + "': " +
                (reason ? reason : "unknown image error");
        return false;
    }

    sg_image_desc imageDescriptor{};
    imageDescriptor.width = width;
    imageDescriptor.height = height;
    imageDescriptor.pixel_format = SG_PIXELFORMAT_RGBA8;
    imageDescriptor.data.mip_levels[0].ptr = pixels;
    imageDescriptor.data.mip_levels[0].size =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U;
    imageDescriptor.label = "particle-editor-preview-texture";
    const sg_image candidateImage = sg_make_image(&imageDescriptor);
    stbi_image_free(pixels);

    if (sg_query_image_state(candidateImage) != SG_RESOURCESTATE_VALID) {
        if (candidateImage.id != 0) sg_destroy_image(candidateImage);
        error = "GPU rejected texture '" + displayPath + "'";
        return false;
    }

    sg_view_desc viewDescriptor{};
    viewDescriptor.texture.image = candidateImage;
    viewDescriptor.label = "particle-editor-preview-view";
    const sg_view candidateView = sg_make_view(&viewDescriptor);
    if (sg_query_view_state(candidateView) != SG_RESOURCESTATE_VALID) {
        if (candidateView.id != 0) sg_destroy_view(candidateView);
        sg_destroy_image(candidateImage);
        error = "GPU rejected texture view for '" + displayPath + "'";
        return false;
    }

    reset();
    image_ = candidateImage;
    view_ = candidateView;
    width_ = width;
    height_ = height;
    error.clear();
    return true;
}

void PreviewTexture::reset() noexcept {
    if (view_.id != 0) sg_destroy_view(view_);
    if (image_.id != 0) sg_destroy_image(image_);
    view_ = {};
    image_ = {};
    width_ = 0;
    height_ = 0;
}

std::uint64_t PreviewTexture::imguiId() const noexcept {
    return view_.id != 0 ? simgui_imtextureid(view_) : 0;
}

}  // namespace newaoe::render::editor

