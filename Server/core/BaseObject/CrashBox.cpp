#include "Server/core/BaseObject/CrashBox.h"

namespace {

// 构造阶段一次性处理非法负值，热路径中无需重复判断。
[[nodiscard]] FORCE_INLINE U32 NonNegative(I32 value) noexcept {
    return value > 0 ? static_cast<U32>(value) : U32{0};
}

// 所有边界和圆心先扩展到 I64，覆盖完整 I32 坐标与尺寸范围。
[[nodiscard]] FORCE_INLINE U64 AxisDistance(I64 lhs, I64 rhs) noexcept {
    const I64 delta = lhs - rhs;
    return static_cast<U64>(delta < 0 ? -delta : delta);
}

[[nodiscard]] FORCE_INLINE I64 FarEdge(I32 position, U32 size) noexcept {
    return static_cast<I64>(position) + size;
}

[[nodiscard]] FORCE_INLINE U64 DistanceOutsideBounds(I64 value,
                                                     I64 minimum,
                                                     I64 maximum) noexcept {
    return value < minimum
        ? static_cast<U64>(minimum - value)
        : (value > maximum
               ? static_cast<U64>(value - maximum)
               : U64{0});
}

[[nodiscard]] FORCE_INLINE bool IsValidRect(U32 width,
                                            U32 height) noexcept {
    return width != 0 && height != 0;
}

// 先按轴快速排除，再用减法比较平方，避免 dx^2 + dy^2 溢出。
[[nodiscard]] FORCE_INLINE bool IsWithinRadius(U64 distanceX,
                                               U64 distanceY,
                                               U64 radius) noexcept {
    if (distanceX > radius || distanceY > radius) {
        return false;
    }
    if (distanceX == 0 || distanceY == 0) {
        return true;
    }

    const U64 radiusSquared = radius * radius;
    const U64 distanceXSquared = distanceX * distanceX;
    return distanceY * distanceY <= radiusSquared - distanceXSquared;
}

[[nodiscard]] FORCE_INLINE bool IsRectCircle(IVec2 rectPos,
                                             U32 width,
                                             U32 height,
                                             IVec2 circlePos,
                                             U32 radius) noexcept {
    if (radius == 0 || !IsValidRect(width, height)) {
        return false;
    }

    const I64 rectLeft = rectPos.x;
    const I64 rectTop = rectPos.y;
    const I64 circleCenterX = circlePos.x;
    const I64 circleCenterY = circlePos.y;
    return IsWithinRadius(
        DistanceOutsideBounds(circleCenterX,
                              rectLeft,
                              FarEdge(rectPos.x, width)),
        DistanceOutsideBounds(circleCenterY,
                              rectTop,
                              FarEdge(rectPos.y, height)),
        radius);
}

} // namespace

CrashBox_Rect::CrashBox_Rect(IVec2 detailPos,
                             I32 detailL,
                             I32 detailU) noexcept
    : detailPos_(detailPos),
      detailW_(NonNegative(detailL)),
      detailH_(NonNegative(detailU)) {}

CrashBox_Circle::CrashBox_Circle(IVec2 detailPos, I32 rad) noexcept
    : detailPos_(detailPos), rad_(NonNegative(rad)) {}

IVec2 CrashBox_Rect::GetPos() const noexcept {
    return detailPos_;
}

U32 CrashBox_Rect::GetW() const noexcept {
    return detailW_;
}

U32 CrashBox_Rect::GetH() const noexcept {
    return detailH_;
}

IVec2 CrashBox_Circle::GetPos() const noexcept {
    return detailPos_;
}

U32 CrashBox_Circle::GetR() const noexcept {
    return rad_;
}

bool CrashBox_Rect::IsCrash(const CrashBox_Rect& other) const noexcept {
    if (!IsValidRect(detailW_, detailH_) ||
        !IsValidRect(other.detailW_, other.detailH_)) {
        return false;
    }

    const I64 left = detailPos_.x;
    const I64 top = detailPos_.y;
    const I64 otherLeft = other.detailPos_.x;
    const I64 otherTop = other.detailPos_.y;
    return left <= FarEdge(other.detailPos_.x, other.detailW_) &&
           FarEdge(detailPos_.x, detailW_) >= otherLeft &&
           top <= FarEdge(other.detailPos_.y, other.detailH_) &&
           FarEdge(detailPos_.y, detailH_) >= otherTop;
}

bool CrashBox_Rect::IsCrash(const CrashBox_Circle& other) const noexcept {
    return IsRectCircle(detailPos_,
                        detailW_,
                        detailH_,
                        other.detailPos_,
                        other.rad_);
}

bool CrashBox_Circle::IsCrash(const CrashBox_Circle& other) const noexcept {
    if (rad_ == 0 || other.rad_ == 0) {
        return false;
    }

    const U64 radius = static_cast<U64>(rad_) + other.rad_;
    return IsWithinRadius(AxisDistance(detailPos_.x, other.detailPos_.x),
                          AxisDistance(detailPos_.y, other.detailPos_.y),
                          radius);
}

bool CrashBox_Circle::IsCrash(const CrashBox_Rect& other) const noexcept {
    return IsRectCircle(other.detailPos_,
                        other.detailW_,
                        other.detailH_,
                        detailPos_,
                        rad_);
}
