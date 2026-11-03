#pragma once

#include "Public/vec/vec.hpp"

/******************************碰撞盒类型*************************/
enum CrashBoxType {
    BOX_CIRCLE = 0, // 圆形碰撞盒
    BOX_RECT        // 矩形碰撞盒
};

class CrashBox_Circle;

// detailPos 是左上角；detailL/detailU 分别是完整宽度和完整高度。
// 任一尺寸为 0 时，该碰撞盒永远不参与碰撞。
class CrashBox_Rect final {
public:
    static constexpr CrashBoxType Type = BOX_RECT;

    CrashBox_Rect(IVec2 detailPos, I32 detailL, I32 detailU) noexcept;

    [[nodiscard]] IVec2 GetPos() const noexcept;
    [[nodiscard]] U32 GetW() const noexcept;
    [[nodiscard]] U32 GetH() const noexcept;

    [[nodiscard]] bool IsCrash(const CrashBox_Rect& other) const noexcept;

    [[nodiscard]] bool IsCrash(const CrashBox_Circle& other) const noexcept;

private:
    friend class CrashBox_Circle;

    IVec2 detailPos_;
    U32 detailW_;
    U32 detailH_;
};

// detailPos 是圆心；rad 是半径。rad 为 0 时永远不参与碰撞。
class CrashBox_Circle final {
public:
    static constexpr CrashBoxType Type = BOX_CIRCLE;

    CrashBox_Circle(IVec2 detailPos, I32 rad) noexcept;

    [[nodiscard]] IVec2 GetPos() const noexcept;
    [[nodiscard]] U32 GetR() const noexcept;

    [[nodiscard]] bool IsCrash(const CrashBox_Circle& other) const noexcept;

    [[nodiscard]] bool IsCrash(const CrashBox_Rect& other) const noexcept;

private:
    friend class CrashBox_Rect;

    IVec2 detailPos_;
    U32 rad_;
};

static_assert(sizeof(CrashBox_Rect) == sizeof(I32) * 4,
              "CrashBox_Rect must stay compact");
static_assert(sizeof(CrashBox_Circle) == sizeof(I32) * 3,
              "CrashBox_Circle must stay compact");

