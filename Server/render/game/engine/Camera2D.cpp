#include "Server/render/game/engine/Camera2D.h"

#include <algorithm>
#include <cmath>

namespace newaoe::render {

void Camera2D::translate(Vec2 delta) noexcept {
    position_.x += delta.x;
    position_.y += delta.y;
    positionExplicit_ = true;
}

void Camera2D::setZoom(float value) noexcept {
    zoom_ = std::max(value, 0.0001F);
}

Matrix4 Camera2D::viewProjection(float viewportWidth,
                                 float viewportHeight) const noexcept {
    const float width = std::max(viewportWidth, 1.0F);
    const float height = std::max(viewportHeight, 1.0F);
    const float cosine = std::cos(rotation_);
    const float sine = std::sin(rotation_);
    const float scaleX = 2.0F * zoom_ / width;
    const float scaleY = -2.0F * zoom_ / height;

    // Column-major matrix.  Camera rotation is the inverse transform of the
    // world, while the negative Y scale preserves screen-style Y-down space.
    const float m00 = cosine * scaleX;
    const float m01 = -sine * scaleY;
    const float m10 = sine * scaleX;
    const float m11 = cosine * scaleY;
    const float tx = -(m00 * position_.x + m10 * position_.y);
    const float ty = -(m01 * position_.x + m11 * position_.y);

    return Matrix4{
        m00, m01, 0.0F, 0.0F,
        m10, m11, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        tx, ty, 0.0F, 1.0F,
    };
}

}  // namespace newaoe::render
