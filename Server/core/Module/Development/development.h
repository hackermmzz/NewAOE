#pragma once

#include <array>

#include "Server/core/type.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Public/config/unit_attribute.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Building.h"

class Player;
class Object;

// ============================================================================
// 科技位图机制
// ============================================================================
struct TechnologyBits
{
    U64 low;
    U64 high;

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

    [[nodiscard]] bool contains(const TechnologyBits& other) const
    {
        return (low & other.low) == other.low && (high & other.high) == other.high;
    }

    [[nodiscard]] bool empty() const
    {
        return low == 0 && high == 0;
    }
};

constexpr TechnologyBits techBit(I32 technology)
{
    return (technology <= 0 || technology >= 128)
        ? TechnologyBits()
        : (technology < 64
            ? TechnologyBits(U64(1) << technology, 0)
            : TechnologyBits(0, U64(1) << (technology - 64)));
}

constexpr TechnologyBits operator|(TechnologyBits lhs, TechnologyBits rhs)
{
    return TechnologyBits(lhs.low | rhs.low, lhs.high | rhs.high);
}
// ============================================================================


class Development
{
public:
    Development(WeakPtr<Player> player, I32 civilization);
    ~Development() = default;

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
    [[nodiscard]] std::array<I32,2> get_speed_attack(I32 sort, I32 type, I32 armyClass) const;
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

    void notifyBuildingActionFinish(WeakPtr<Object> object, I32 action);
    void tick();

private:
    WeakPtr<Player> player;
    I32 stage;
    I32 civilization;
    
    TechnologyBits technology;
    TechnologyBits blocked;
    U64 blockedBuilding;
};