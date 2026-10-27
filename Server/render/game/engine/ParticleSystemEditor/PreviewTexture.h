#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "sokol_gfx.h"

namespace newaoe::render::editor {

class PreviewTexture final {
public:
    PreviewTexture() = default;
    ~PreviewTexture();

    PreviewTexture(const PreviewTexture&) = delete;
    PreviewTexture& operator=(const PreviewTexture&) = delete;

    [[nodiscard]] bool load(const std::filesystem::path& path,
                            std::string& error);
    void reset() noexcept;

    [[nodiscard]] std::uint64_t imguiId() const noexcept;
    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }

private:
    sg_image image_{};
    sg_view view_{};
    int width_ = 0;
    int height_ = 0;
};

}  // namespace newaoe::render::editor

