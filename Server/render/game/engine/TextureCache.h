#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Server/render/game/engine/RenderTypes.h"
#include "sokol_gfx.h"

namespace newaoe::render::detail {

struct TextureGpuResource final {
    sg_image image{};
    sg_view view{};
    int width = 0;
    int height = 0;
};

class TextureCache final {
public:
    bool initialize(std::string& error);
    void shutdown() noexcept;

    TextureHandle load(std::string_view path, std::string& error,
                       bool srgb = true);
    bool release(TextureHandle handle) noexcept;
    [[nodiscard]] const TextureGpuResource* resolve(
        TextureHandle handle) const noexcept;

    [[nodiscard]] sg_sampler sampler() const noexcept { return sampler_; }
    [[nodiscard]] sg_view whiteView() const noexcept { return white_.view; }
    [[nodiscard]] sg_view flatNormalView() const noexcept {
        return flatNormal_.view;
    }
    [[nodiscard]] sg_view blackView() const noexcept { return black_.view; }

private:
    struct Slot final {
        TextureGpuResource gpu{};
        std::string canonicalPath;
        std::string cacheKey;
        std::uint32_t generation = 1;
        std::uint32_t references = 0;
        bool occupied = false;
    };

    static bool createSolidTexture(std::uint32_t rgba,
                                   TextureGpuResource& output,
                                   const char* label,
                                   std::string& error);
    static std::string canonicalize(std::string_view path);
    static void destroy(TextureGpuResource& resource) noexcept;

    std::vector<Slot> slots_;
    std::vector<std::uint32_t> freeSlots_;
    std::unordered_map<std::string, std::uint32_t> byPath_;
    sg_sampler sampler_{};
    TextureGpuResource white_{};
    TextureGpuResource flatNormal_{};
    TextureGpuResource black_{};
};

}  // namespace newaoe::render::detail
