#pragma once

#include <array>

#include "Server/core/type.h"
#include "Server/core/BaseObject/WeakPointer.hpp"

class Player;
class Object;

// ============================================================================
// 科技位图机制
// 使用两个 U64 模拟 128 位位图，用于极高性能地记录和查询已研发的科技，
// 以及记录不同文明被封锁（无法研发）的科技。
// ============================================================================
struct TechnologyBits
{
    U64 low;    // 位 0 ~ 63
    U64 high;   // 位 64 ~ 127

    constexpr TechnologyBits() : low(0), high(0) {}
    constexpr TechnologyBits(U64 lowBits, U64 highBits) : low(lowBits), high(highBits) {}

    void add(I32 technology)
    {
        if (technology <= 0 || technology >= 128) return;
        if (technology < 64) {
            low |= (U64(1) << technology);
        } else {
            high |= (U64(1) << (technology - 64));
        }
    }

    [[nodiscard]] bool has(I32 technology) const
    {
        if (technology <= 0 || technology >= 128) return false;
        if (technology < 64) {
            return (low & (U64(1) << technology)) != 0;
        }
        return (high & (U64(1) << (technology - 64))) != 0;
    }

    // this 是否包含 other 里的全部位（用于前置科技判定）
    [[nodiscard]] bool contains(const TechnologyBits& other) const
    {
        return (low & other.low) == other.low && (high & other.high) == other.high;
    }

    [[nodiscard]] bool empty() const
    {
        return low == 0 && high == 0;
    }
};

// 把科技 ID 转成只有这一位的位图
constexpr TechnologyBits techBit(I32 technology)
{
    return (technology <= 0 || technology >= 128)
        ? TechnologyBits()
        : (technology < 64
            ? TechnologyBits(U64(1) << technology, 0)
            : TechnologyBits(0, U64(1) << (technology - 64)));
}

// 重载 | 运算符，支持 techBit(A) | techBit(B)
constexpr TechnologyBits operator|(TechnologyBits lhs, TechnologyBits rhs)
{
    return TechnologyBits(lhs.low | rhs.low, lhs.high | rhs.high);
}
// ============================================================================


// 玩家的科技/发展状态。
class Development
{
public:
    Development(WeakPtr<Player> player, I32 civilization);
    ~Development() = default;

    // ---- 科技状态记录与查询 ----
    bool markTechnologyFinish(I32 action);
    [[nodiscard]] bool TechnologyFinish(I32 technology) const;
    [[nodiscard]] bool TechnologyBlocked(I32 technology) const;

    // ---- 加成查询 ----
    [[nodiscard]] Double get_rate_Move(I32 sort, I32 type) const;
    [[nodiscard]] Double get_rate_Blood(I32 sort, I32 type) const;
    [[nodiscard]] I32 get_addition_Blood(I32 sort, I32 type) const;
    [[nodiscard]] Double get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;
    [[nodiscard]] I32 get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;
    [[nodiscard]] I32 get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;
    [[nodiscard]] Double get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const;
    [[nodiscard]] I32 get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const;
    [[nodiscard]] I32 get_addition_ResourceCarry(I32 resourceSort) const;
    [[nodiscard]] Double get_rate_ResourceGather(I32 resourceSort) const;
    [[nodiscard]] I32 get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const;

    // ---- 时代 ----
    [[nodiscard]] bool stageCanChange() const;

    // ---- 建筑 ----
    [[nodiscard]] bool buildingCanBuild(I32 buildingType) const;
    [[nodiscard]] bool buildingActionAble(I32 buildingType, I32 actNum) const;
    [[nodiscard]] I32 buildingBuildTimeCost(I32 buildingType) const;
    [[nodiscard]] std::array<I32,4> buildingResourceCost(I32 buildingType) const;
    [[nodiscard]] I32 buildingActionTimeCost(I32 buildingType, I32 actNum) const;
    [[nodiscard]] std::array<I32,4> buildingActionResourceCost(I32 buildingType, I32 actNum) const;

    // 内核通知：某个建筑动作已成功完成。
    void notifyBuildingActionFinish(WeakPtr<Object> object, I32 action);

    // 每次完整运行完一次内核后执行
    void tick();

private:
    WeakPtr<Player> player;    // 只读 player，不得修改
    I32 stage;                 // 当前阶段
    I32 civilization;          // 文明类型
    
    // 状态位图
    TechnologyBits technology; // 已经完成的科技
    TechnologyBits blocked;    // 本文明被封锁的科技
    U64 blockedBuilding;       // 本文明不能建造的建筑（按位索引）
};