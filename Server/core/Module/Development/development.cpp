#include "development.h"

#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/BaseObject/Object.h"
#include "Public/logger/logger.h"

// 说明：本类目前只掌握 stage / civilization 两个状态量，工程中还没有
// 任何「文明/时代 → 加成、资源消耗、耗时、解锁条件」的配置表
// （Public/config/、Public/var/ 下均无对应数据）。因此凡需要这类数值的
// 接口，一律先输出缺失配置日志，再返回规定默认值，不做任何推测填值。

Development::Development(WeakPtr<Player> playerValue, I32 civilizationValue)
    : player(playerValue),
      stage(CIVILIZATION_STONEAGE),
      civilization(civilizationValue)
{
}

// ---- 加成查询 ----

Double Development::get_rate_Move(I32 sort, I32 type) const
{
    (void)sort;
    (void)type;
    LOGGER_WARN("缺失配置: 移动速度加成比例表");
    return Double(1);
}

Double Development::get_rate_Blood(I32 sort, I32 type) const
{
    (void)sort;
    (void)type;
    LOGGER_WARN("缺失配置: 血量加成比例表");
    return Double(1);
}

I32 Development::get_addition_Blood(I32 sort, I32 type) const
{
    (void)sort;
    (void)type;
    LOGGER_WARN("缺失配置: 血量数值加成表");
    return 0;
}

Double Development::get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    (void)sort;
    (void)type;
    (void)armyClass;
    (void)attackType;
    LOGGER_WARN("缺失配置: 攻击加成比例表");
    return Double(1);
}

I32 Development::get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    (void)sort;
    (void)type;
    (void)armyClass;
    (void)attackType;
    LOGGER_WARN("缺失配置: 攻击数值加成表");
    return 0;
}

I32 Development::get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    (void)sort;
    (void)type;
    (void)armyClass;
    (void)attackType;
    LOGGER_WARN("缺失配置: 攻击距离加成表");
    return 0;
}

Double Development::get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    (void)sort;
    (void)type;
    (void)armyClass;
    (void)attackType_got;
    LOGGER_WARN("缺失配置: 防御加成比例表");
    return Double(1);
}

I32 Development::get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    (void)sort;
    (void)type;
    (void)armyClass;
    (void)attackType_got;
    LOGGER_WARN("缺失配置: 防御数值加成表");
    return 0;
}

I32 Development::get_addition_ResourceCarry(I32 resourceSort) const
{
    (void)resourceSort;
    LOGGER_WARN("缺失配置: 资源携带量加成表");
    return 0;
}

Double Development::get_rate_ResourceGather(I32 resourceSort) const
{
    (void)resourceSort;
    LOGGER_WARN("缺失配置: 资源采集速度加成比例表");
    return Double(1);
}

I32 Development::get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const
{
    (void)sort;
    (void)type;
    LOGGER_WARN("缺失配置: 资源型建筑产量加成表");
    return 0;
}

// ---- 时代 ----

bool Development::stageCanChange() const
{
    // 时代升级的前置条件与资源消耗没有配置表，无法判定。
    LOGGER_WARN("缺失配置: 时代升级条件");
    return false;
}

// ---- 建筑 ----

bool Development::buildingCanBuild(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    // 建筑的文明/时代解锁条件没有配置表。
    LOGGER_WARN("缺失配置: 建筑解锁条件表(文明/时代)");
    return false;
}

bool Development::buildingActionAble(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    (void)actNum;
    // 建筑动作的解锁条件、次数上限、时代前置没有配置表。
    LOGGER_WARN("缺失配置: 建筑动作解锁条件表");
    return false;
}

I32 Development::buildingBuildTimeCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return 0;
    }
    LOGGER_WARN("缺失配置: 建筑建造耗时表");
    return 0;
}

std::array<I32,4> Development::buildingResourceCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return {0, 0, 0, 0};
    }
    LOGGER_WARN("缺失配置: 建筑建造成本表");
    return {0, 0, 0, 0};
}

I32 Development::buildingActionTimeCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return 0;
    }
    (void)actNum;
    LOGGER_WARN("缺失配置: 建筑动作耗时表");
    return 0;
}

std::array<I32,4> Development::buildingActionResourceCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return {0, 0, 0, 0};
    }
    (void)actNum;
    LOGGER_WARN("缺失配置: 建筑动作资源消耗表");
    return {0, 0, 0, 0};
}

void Development::notifyBuildingActionFinish(WeakPtr<Object> object, I32 action)
{
    // 只读 player，本函数不改动任何对象。
    (void)object;
    // 时代升级动作到阶段编号的映射直接取自 BuildingAction 与
    // NEW_CIVILIZATION_STAGE 两个既有枚举，不需要额外的数值配置。
    switch (action) {
    case BUILDING_CENTER_UPGRADE_TOOLAGE:
        if (stage < CIVILIZATION_TOOLAGE) {
            stage = CIVILIZATION_TOOLAGE;
        }
        break;
    case BUILDING_CENTER_UPGRADE_BRONZEAGE:
        if (stage < CIVILIZATION_BRONZEAGE) {
            stage = CIVILIZATION_BRONZEAGE;
        }
        break;
    case BUILDING_CENTER_UPGRADE_IRONAGE:
        if (stage < CIVILIZATION_IRONAGE) {
            stage = CIVILIZATION_IRONAGE;
        }
        break;
    default:
        // 其余动作（研究/解锁/造人）目前没有需要在本类维护的状态：
        // 私有成员只有 player / stage / civilization，无法记录已完成科技。
        break;
    }
}

void Development::tick()
{
    // 私有成员只有 player / stage / civilization，且 player 只读，
    // 没有需要逐帧推进的状态。
}
