#include "development.h"

#include "Server/core/Public/var/Building.h"

namespace {
// TODO: 待接入时代配置表后改为查表；当前按石器、工具、铜器、铁器 4 个时代处理
constexpr I32 STAGE_MIN = 0;
constexpr I32 STAGE_MAX = 3;
}

Development::Development(WeakPtr<Player> playerValue, I32 civilizationValue)
    : player(playerValue),
      stage(STAGE_MIN),
      civilization(civilizationValue)
{
}

// ---- 加成查询 ----

Double Development::get_rate_Move(I32 sort, I32 type) const
{
    // TODO: 待设计配置表后填入具体加成
    return Double(1);
}

Double Development::get_rate_Blood(I32 sort, I32 type) const
{
    // TODO: 待设计配置表后填入具体加成
    return Double(1);
}

I32 Development::get_addition_Blood(I32 sort, I32 type) const
{
    // TODO: 待设计配置表后填入具体加成
    return 0;
}

Double Development::get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    // TODO: 待设计配置表后填入具体加成
    return Double(1);
}

I32 Development::get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    // TODO: 待设计配置表后填入具体加成
    return 0;
}

I32 Development::get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    // TODO: 待设计配置表后填入具体加成
    return 0;
}

Double Development::get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    // TODO: 待设计配置表后填入具体加成
    return Double(1);
}

I32 Development::get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    // TODO: 待设计配置表后填入具体加成
    return 0;
}

I32 Development::get_addition_ResourceCarry(I32 resourceSort) const
{
    // TODO: 待设计配置表后填入具体加成
    return 0;
}

Double Development::get_rate_ResourceGather(I32 resourceSort) const
{
    // TODO: 待设计配置表后填入具体加成
    return Double(1);
}

I32 Development::get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const
{
    // TODO: 待设计配置表后填入具体加成
    return 0;
}

// ---- 时代 ----

void Development::stageChange()
{
    if (stage >= STAGE_MAX) {
        return;
    }
    stage += 1;
}

bool Development::stageCanChange() const
{
    // TODO: 待确认时代升级的前置条件和资源消耗
    return false;
}

// ---- 建筑 ----

bool Development::buildingActionAble(I32 buildingType, I32 actNum,
                                     I32 wood, I32 food, I32 stone, I32 gold) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    if (actNum <= 0) {
        return false;
    }
    if (wood < 0 || food < 0 || stone < 0 || gold < 0) {
        return false;
    }

    // 示例：市镇中心训练农民需要消耗食物
    if (buildingType == BUILDING_CENTER && actNum == BUILDING_CENTER_CREATEFARMER) {
        // TODO: 待设计配置表后填入具体资源消耗
        return food > 0;
    }

    // 示例：市镇中心升级要求先进入下一个时代
    if (buildingType == BUILDING_CENTER && actNum == BUILDING_CENTER_UPGRADE) {
        // TODO: 待确认时代升级的前置条件和资源消耗
        return true;
    }

    // TODO: 待设计配置表后补充其余建筑动作的时代/资源前置条件
    return true;
}

I32 Development::buildingActionTimeCost(I32 buildingType, I32 actNum) const
{
    // TODO: 待设计时间/帧数配置表后填入实际耗时
    return 0;
}

bool Development::buildingCanBuild(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    if (civilization < 0) {
        return false;
    }

    switch (buildingType) {
    case BUILDING_TEMPLE:
    case BUILDING_WALL:
        // 这两个建筑尚未实现
        return false;
    case BUILDING_SIEGE:
        // 示例：高级建筑要求进入工具时代之后
        // TODO: 待设计文明/时代解锁配置表后确认具体时代
        return stage > STAGE_MIN;
    default:
        // TODO: 待设计文明/时代解锁配置表后补充具体条件
        return true;
    }
}

I32 Development::buildingBuildTimeCost(I32 buildingType) const
{
    // TODO: 待设计时间/帧数配置表后填入实际耗时
    return 0;
}

void Development::tick()
{
    // TODO: 待实现帧同步更新逻辑
}