#pragma once

#include "Server/render/game/engine/RenderTypes.h"

namespace newaoe::render {

// Orthographic, Y-down camera for 2D world coordinates.  At zoom 1, one world
// unit covers one framebuffer pixel.  Position denotes the viewport centre.
class Camera2D final {
public:
    void setPosition(Vec2 value) noexcept {
        position_ = value;
        positionExplicit_ = true;
    }
    [[nodiscard]] Vec2 position() const noexcept { return position_; }
    void translate(Vec2 delta) noexcept;

    void setRotation(float radians) noexcept { rotation_ = radians; }
    [[nodiscard]] float rotation() const noexcept { return rotation_; }
    void rotate(float radians) noexcept { rotation_ += radians; }

    void setZoom(float value) noexcept;
    [[nodiscard]] float zoom() const noexcept { return zoom_; }

    [[nodiscard]] Matrix4 viewProjection(float viewportWidth,
                                         float viewportHeight) const noexcept;

private:
    friend class RenderEngine;

    // Keeps the zero-configuration camera centred on the viewport while
    // never overwriting a position chosen by game code.
    void setDefaultPosition(Vec2 value) noexcept {
        if (!positionExplicit_) position_ = value;
    }

    Vec2 position_{};
    float rotation_ = 0.0F;
    float zoom_ = 1.0F;
    bool positionExplicit_ = false;
};

}  // namespace newaoe::render
