#include "development.h"

#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/BaseObject/Object.h"
#include "Public/logger/logger.h"

#define BLD_BIT(x) (x >= 0 && x < 64 ? (U64(1) << x) : 0)

Development::Development(WeakPtr<Player> playerValue, I32 civilizationValue)
    : player(playerValue),
      stage(CIVILIZATION_STONEAGE),
      civilization(civilizationValue),
      technology(),
      blocked(),
      blockedBuilding(0)
{
    // TODO: 接入外置配置表后，根据 civilization 初始化文明封锁掩码
}

// ---- 科技状态 ----

bool Development::markTechnologyFinish(I32 action)
{
    // 纯枚举拦截：造人动作不是科技，不记录位图
    if (action == BUILDING_CENTER_CREATEFARMER ||
        action == BUILDING_ARMYCAMP_CREATE_CLUBMAN ||
        action == BUILDING_ARMYCAMP_CREATE_SLINGER ||
        action == BUILDING_RANGE_CREATE_BOWMAN ||
        action == BUILDING_STABLE_CREATE_SCOUT ||
        action == BUILDING_DOCK_CREATE_SAILING ||
        action == BUILDING_TEMPLE_CREATE_PRIEST)
    {
        return false;
    }

    if (technology.has(action) || TechnologyBlocked(action)) {
        return false;
    }
    
    technology.add(action);

    // 纯枚举：时代推进
    if (action == BUILDING_CENTER_UPGRADE_TOOLAGE && stage < CIVILIZATION_TOOLAGE) {
        stage = CIVILIZATION_TOOLAGE;
    } else if (action == BUILDING_CENTER_UPGRADE_BRONZEAGE && stage < CIVILIZATION_BRONZEAGE) {
        stage = CIVILIZATION_BRONZEAGE;
    } else if (action == BUILDING_CENTER_UPGRADE_IRONAGE && stage < CIVILIZATION_IRONAGE) {
        stage = CIVILIZATION_IRONAGE;
    }
    return true;
}

bool Development::TechnologyFinish(I32 technologyId) const
{
    return technology.has(technologyId);
}

bool Development::TechnologyBlocked(I32 technologyId) const
{
    return blocked.has(technologyId);
}

// ---- 加成查询 (表未导入，打警告，返回默认) ----
Double Development::get_rate_Move(I32 sort, I32 type) const { (void)sort; (void)type; LOGGER_WARN("缺失配置: 移动速度加成"); return Double(1); }
Double Development::get_rate_Blood(I32 sort, I32 type) const { (void)sort; (void)type; LOGGER_WARN("缺失配置: 血量加成"); return Double(1); }
I32 Development::get_addition_Blood(I32 sort, I32 type) const { (void)sort; (void)type; LOGGER_WARN("缺失配置: 血量数值加成"); return 0; }
Double Development::get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const { (void)sort; (void)type; (void)armyClass; (void)attackType; LOGGER_WARN("缺失配置: 攻击加成"); return Double(1); }
I32 Development::get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const { (void)sort; (void)type; (void)armyClass; (void)attackType; LOGGER_WARN("缺失配置: 攻击数值加成"); return 0; }
I32 Development::get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const { (void)sort; (void)type; (void)armyClass; (void)attackType; LOGGER_WARN("缺失配置: 攻击距离加成"); return 0; }
Double Development::get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const { (void)sort; (void)type; (void)armyClass; (void)attackType_got; LOGGER_WARN("缺失配置: 防御加成"); return Double(1); }
I32 Development::get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const { (void)sort; (void)type; (void)armyClass; (void)attackType_got; LOGGER_WARN("缺失配置: 防御数值加成"); return 0; }
I32 Development::get_addition_ResourceCarry(I32 resourceSort) const { (void)resourceSort; LOGGER_WARN("缺失配置: 携带量加成"); return 0; }
Double Development::get_rate_ResourceGather(I32 resourceSort) const { (void)resourceSort; LOGGER_WARN("缺失配置: 采集速度加成"); return Double(1); }
I32 Development::get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const { (void)sort; (void)type; LOGGER_WARN("缺失配置: 资源建筑产量加成"); return 0; }

// ---- 时代 ----

bool Development::stageCanChange() const
{
    // 纯枚举：定位下一个时代的升级动作
    I32 nextAction = -1;
    switch (stage) {
    case CIVILIZATION_STONEAGE:  nextAction = BUILDING_CENTER_UPGRADE_TOOLAGE;  break;
    case CIVILIZATION_TOOLAGE:   nextAction = BUILDING_CENTER_UPGRADE_BRONZEAGE; break;
    case CIVILIZATION_BRONZEAGE: nextAction = BUILDING_CENTER_UPGRADE_IRONAGE;  break;
    default: return false;
    }

    if (TechnologyBlocked(nextAction)) {
        return false;
    }

    // TODO: 接入配置表，判断当前玩家是否满足 nextAction 的前置科技/建筑/资源要求（表中已有 TOOL_AGE_COST_FOOD 等）
    LOGGER_WARN("缺失配置: 时代升级条件");
    return false;
}

// ---- 建筑 ----

bool Development::buildingCanBuild(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    // 文明封锁判定（位图）
    if ((blockedBuilding & BLD_BIT(buildingType)) != 0) {
        return false;
    }

    // TODO: 接入配置表。
    // 你的 Buildings 表里已有 Age（时代）、Cost（花费）、Requires（前置建筑/前置科技）。
    // 直接按 buildingType 查表，判断当前 stage 和前置建筑是否满足。
    LOGGER_WARN("缺失配置: 建筑解锁条件表(文明/时代/前置科技)");
    return false;
}

bool Development::buildingActionAble(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    // 文明封锁判定（位图）
    if (TechnologyBlocked(actNum)) {
        return false;
    }

    // 纯枚举：动作归属建筑限制（表里未记录动作归属哪个建筑，故用枚举写死）
    if (actNum == BUILDING_CENTER_CREATEFARMER && buildingType != BUILDING_CENTER) return false;
    if (actNum == BUILDING_ARMYCAMP_CREATE_CLUBMAN && buildingType != BUILDING_ARMYCAMP) return false;
    if (actNum == BUILDING_RANGE_CREATE_BOWMAN && buildingType != BUILDING_RANGE) return false;
    if (actNum == BUILDING_STABLE_CREATE_CAVALRY && buildingType != BUILDING_STABLE) return false;
    if (actNum == BUILDING_TEMPLE_CREATE_PRIEST && buildingType != BUILDING_TEMPLE) return false;
    if (actNum == BUILDING_DOCK_CREATE_SAILING && buildingType != BUILDING_DOCK) return false;

    // TODO: 接入配置表，判断动作的前置科技和时代要求（表里已包含时代限制）
    // 例如：造弓手需工具时代，可从表里查证，此处只做兜底。
    return true;
}

// ---- 耗时与资源 (表未导入，打警告，返回默认) ----

I32 Development::buildingBuildTimeCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return 0;
    LOGGER_WARN("缺失配置: 建筑建造耗时表");
    return 0;
}

std::array<I32,4> Development::buildingResourceCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return {0, 0, 0, 0};
    LOGGER_WARN("缺失配置: 建筑建造成本表");
    return {0, 0, 0, 0};
}

I32 Development::buildingActionTimeCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return 0;
    (void)actNum;
    LOGGER_WARN("缺失配置: 建筑动作耗时表");
    return 0;
}

std::array<I32,4> Development::buildingActionResourceCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return {0, 0, 0, 0};
    (void)actNum;
    LOGGER_WARN("缺失配置: 建筑动作资源消耗表");
    return {0, 0, 0, 0};
}

void Development::notifyBuildingActionFinish(WeakPtr<Object> object, I32 action)
{
    (void)object;
    markTechnologyFinish(action);
}

void Development::tick()
{
}