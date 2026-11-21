#include "development.h"

#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Sort.h"
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
    // 【之后管】: TODO: 根据文明类型，初始化 blocked 和 blockedBuilding
    // 例如：埃及没有战车弓兵、亚述没有铁盾等。
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

bool Development::TechnologyFinish(I32 technologyId) const { return technology.has(technologyId); }
bool Development::TechnologyBlocked(I32 technologyId) const { return blocked.has(technologyId); }

// ---- 加成查询 ----

Double Development::get_rate_Move(I32 sort, I32 type) const
{
    // 1. 轮子：村民移动速度提升50%
    if (sort == SORT_FARMER && type == FARMERTYPE_FARMER && TechnologyFinish(BUILDING_MARKET_WHEEL_UPGRADE)) {
        return Double(15) / Double(10);
    }
    // 2. 多神教：牧师移动速度提升40%
    if (sort == SORT_ARMY && type == AT_PRIEST && TechnologyFinish(BUILDING_TEMPLE_POLYTHEISM)) {
        return Double(14) / Double(10);
    }
    // 3. 上流社会：学院单位行动速度提升25%
    // 行动速度包含攻击和移动，统一加到移动速度（攻击速度在 get_speed_attack 里处理）
    if (sort == SORT_ARMY && (type == AT_HOPLITE || type == AT_PHALANX || type == AT_CENTURION)) {
        if (TechnologyFinish(BUILDING_GOVERNMENT_ARISTOCRACY)) {
            return Double(125) / Double(100);
        }
    }
    return Double(1);
}

Double Development::get_rate_Blood(I32 sort, I32 type) const
{
    // 贵族：骑兵单位及骆驼兵、两轮战车、镰刀战车、战车弓兵、骑弓兵、重骑弓兵生命值+15%
    if (sort == SORT_ARMY && TechnologyFinish(BUILDING_GOVERNMENT_NOBILITY)) {
        switch (type) {
            case AT_SCOUT: case AT_CAVALRY: case AT_HEAVY_CAVALRY:
            case AT_CATAPHRACT: case AT_CAMEL_RIDER: 
            case AT_CHARIOT: case AT_SCYTHECHARIOT:
            case AT_CHARIOT_ARCHER: case AT_HORSE_ARCHER:
            case AT_HEAVY_HORSE_ARCHER:
                return Double(115) / Double(100);
            default: break;
        }
    }
    return Double(1);
}

I32 Development::get_addition_Blood(I32 sort, I32 type) const
{
    // 神秘主义：牧师生命值翻倍（基础 25 翻倍为 50，相当于额外 +25）
    if (sort == SORT_ARMY && type == AT_PRIEST && TechnologyFinish(BUILDING_TEMPLE_MYSTICISM)) {
        return 25; 
    }
    return 0;
}

Double Development::get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    (void)sort; (void)type; (void)armyClass; (void)attackType;
    return Double(1);
}

I32 Development::get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    I32 result = 0;
    
    // 1. 近战单位攻击提升（工具制造术+2、金属加工术+2、冶金术+3）
    if (attackType == ATTACKTYPE_CLOSE) {
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_USETOOL)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_METALWORKING)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_METALLURGY)) result += 3;
    }

    // 2. 炼金术：远程单位攻击+1
    if (attackType == ATTACKTYPE_SHOOT && TechnologyFinish(BUILDING_GOVERNMENT_ALCHEMY)) {
        result += 1;
    }

    // 3. 炼金术：弩车/巨型弩车攻击+2
    if (type == AT_BALLISTA || type == AT_HELEPOLIS) {
        if (TechnologyFinish(BUILDING_GOVERNMENT_ALCHEMY)) result += 2;
    }

    // 4. 炼金术：喷火船攻击+6
    if (type == AT_FIRESAILBOAT && TechnologyFinish(BUILDING_GOVERNMENT_ALCHEMY)) {
        result += 6;
    }

    // 5. 采石术 / 攻城术：投石兵攻击+1
    if (type == AT_SLINGER) {
        if (TechnologyFinish(BUILDING_MARKET_STONE_UPGRADE)) result += 1;
        if (TechnologyFinish(BUILDING_MARKET_SIEGECRAFT)) result += 1;
    }

    return result;
}

I32 Development::get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    I32 result = 0;

    // 1. 使用木器 / 手工艺 / 工艺学：远程单位射程+1
    if (attackType == ATTACKTYPE_SHOOT) {
        if (TechnologyFinish(BUILDING_MARKET_WOOD_UPGRADE)) result += 1;
        if (TechnologyFinish(BUILDING_MARKET_ARTISANSHIP)) result += 1;
        if (TechnologyFinish(BUILDING_MARKET_CRAFT_UPGRADE)) result += 1;
    }

    // 2. 采石术 / 攻城术：投石兵射程+1
    if (type == AT_SLINGER) {
        if (TechnologyFinish(BUILDING_MARKET_STONE_UPGRADE)) result += 1;
        if (TechnologyFinish(BUILDING_MARKET_SIEGECRAFT)) result += 1;
    }

    // 3. 工程学：攻城武器射程+2
    if (armyClass == ARMY_SIEGE && TechnologyFinish(BUILDING_GOVERNMENT_ENGINEERING)) {
        result += 2;
    }

    // 4. 来世：牧师招降射程+3
    if (type == AT_PRIEST && TechnologyFinish(BUILDING_TEMPLE_AFTERLIFE)) {
        result += 3;
    }
    
    return result;
}

std::array<I32,2> Development::get_speed_attack(I32 sort, I32 type, I32 armyClass) const
{
    // 暂不支持攻击前摇/帧数
    (void)sort; (void)type; (void)armyClass;
    return {0, 0}; 
}

Double Development::get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    (void)sort; (void)type; (void)armyClass; (void)attackType_got;
    return Double(1);
}

I32 Development::get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    I32 result = 0;

    // 1. 步兵：兵营与学院单位护甲+2（皮甲/鳞甲/锁甲）
    if (armyClass == ARMY_INFANTRY) {
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_1)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_2)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_3)) result += 2;

        // 步兵专属：青铜盾 / 铁盾 / 塔盾，对远程护甲+1
        if (attackType_got == ATTACKTYPE_SHOOT) {
            if (TechnologyFinish(BUILDING_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY)) result += 1;
            if (TechnologyFinish(BUILDING_STOCK_UPGRADE_IRON_SHIELD)) result += 1;
            if (TechnologyFinish(BUILDING_STOCK_UPGRADE_TOWER_SHIELD)) result += 1;
        }
    } 
    // 2. 弓箭兵：靶场单位护甲+2
    else if (armyClass == ARMY_ARCHER) {
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_1)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_2)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_3)) result += 2;
    } 
    // 3. 骑兵：马厩单位护甲+2
    else if (armyClass == ARMY_RIDER) {
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_1)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_2)) result += 2;
        if (TechnologyFinish(BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_3)) result += 2;
    }

    return result;
}

I32 Development::get_addition_ResourceCarry(I32 resourceSort) const
{
    (void)resourceSort;
    return 0;
}

Double Development::get_rate_ResourceGather(I32 resourceSort) const
{
    I32 percent = 0;
    if (resourceSort == RESOURCESORT_WOOD) {
        // 使用木器 / 手工艺 / 工艺学：村民伐木效率+2（按 20% 处理）
        if (TechnologyFinish(BUILDING_MARKET_WOOD_UPGRADE)) percent += 20;
        if (TechnologyFinish(BUILDING_MARKET_ARTISANSHIP)) percent += 20;
        if (TechnologyFinish(BUILDING_MARKET_CRAFT_UPGRADE)) percent += 20;
    } else if (resourceSort == RESOURCESORT_STONE) {
        // 采石术 / 攻城术：采石效率+3（按 30% 处理）
        if (TechnologyFinish(BUILDING_MARKET_STONE_UPGRADE)) percent += 30;
        if (TechnologyFinish(BUILDING_MARKET_SIEGECRAFT)) percent += 30;
    } else if (resourceSort == RESOURCESORT_GOLD) {
        // 采金术：采金效率+3（按 30% 处理）
        if (TechnologyFinish(BUILDING_MARKET_GOLD_UPGRADE)) percent += 30;
    }
    return (Double(100) + Double(percent)) / Double(100);
}

I32 Development::get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const
{
    // 饲养 / 犁田 / 灌溉：农田产粮+75 / +100 / +125
    if ((sort == SORT_BUILDING || sort == SORT_BUILDING_RESOURCE) && type == BUILDING_FARM) {
        I32 extra = 0;
        if (TechnologyFinish(BUILDING_MARKET_FARM_UPGRADE)) extra += 75;
        if (TechnologyFinish(BUILDING_MARKET_PLOW_UPGRADE)) extra += 100;
        if (TechnologyFinish(BUILDING_MARKET_IRRIGATION)) extra += 125;
        return extra;
    }
    return 0;
}

// ---- 时代与建筑 ----

bool Development::stageCanChange() const
{
    I32 nextAction = -1;
    I32 cost = 0;
    
    switch (stage) {
    case CIVILIZATION_STONEAGE:  nextAction = BUILDING_CENTER_UPGRADE_TOOLAGE;  cost = TOOL_AGE_COST_FOOD; break;
    case CIVILIZATION_TOOLAGE:   nextAction = BUILDING_CENTER_UPGRADE_BRONZEAGE; cost = BRONZE_AGE_COST_FOOD; break;
    case CIVILIZATION_BRONZEAGE: nextAction = BUILDING_CENTER_UPGRADE_IRONAGE;  cost = IRON_AGE_COST_FOOD; break;
    default: return false;
    }

    if (TechnologyBlocked(nextAction)) return false;
    
    // 【之后管】: TODO: 检查玩家资源是否足够升级
    return cost > 0;
}

bool Development::buildingCanBuild(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return false;
    if ((blockedBuilding & BLD_BIT(buildingType)) != 0) return false;

    // 【之后管】: TODO: 根据配置表判断文明、时代、前置建筑
    switch (buildingType) {
    case BUILDING_CENTER: return TOWN_CENTER_COST_WOOD > 0;
    case BUILDING_HOME: return HOUSE_COST_WOOD > 0;
    case BUILDING_ARMYCAMP: return BARRACKS_COST_WOOD > 0;
    case BUILDING_WATCH_TOWER: return TechnologyFinish(BUILDING_GRANARY_ARROWTOWER);
    case BUILDING_SENTRY_TOWER: return TechnologyFinish(BUILDING_GRANARY_ARROWTOWER_UPGRADE_1);
    case BUILDING_GUARD_TOWER: return TechnologyFinish(BUILDING_GRANARY_ARROWTOWER_UPGRADE_2);
    default: return true;
    }
}

bool Development::buildingActionAble(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return false;
    if (TechnologyBlocked(actNum)) return false;

    if (actNum == BUILDING_CENTER_CREATEFARMER && buildingType != BUILDING_CENTER) return false;
    if (actNum == BUILDING_ARMYCAMP_CREATE_CLUBMAN && buildingType != BUILDING_ARMYCAMP) return false;
    if (actNum == BUILDING_RANGE_CREATE_BOWMAN && buildingType != BUILDING_RANGE) return false;
    if (actNum == BUILDING_STABLE_CREATE_CAVALRY && buildingType != BUILDING_STABLE) return false;
    if (actNum == BUILDING_TEMPLE_CREATE_PRIEST && buildingType != BUILDING_TEMPLE) return false;
    if (actNum == BUILDING_DOCK_CREATE_SAILING && buildingType != BUILDING_DOCK) return false;

    return true;
}

I32 Development::buildingBuildTimeCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return 0;
    switch (buildingType) {
    case BUILDING_CENTER: return TOWN_CENTER_BUILD_TIME;
    case BUILDING_HOME: return HOUSE_BUILD_TIME;
    case BUILDING_ARMYCAMP: return BARRACKS_BUILD_TIME;
    // 【之后管】: TODO: 补全其他建筑的建造时间
    default: return 0;
    }
}

std::array<I32,4> Development::buildingResourceCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return {0, 0, 0, 0};
    switch (buildingType) {
    case BUILDING_CENTER: return { TOWN_CENTER_COST_WOOD, TOWN_CENTER_COST_FOOD, TOWN_CENTER_COST_STONE, TOWN_CENTER_COST_GOLD };
    case BUILDING_HOME: return { HOUSE_COST_WOOD, HOUSE_COST_FOOD, HOUSE_COST_STONE, HOUSE_COST_GOLD };
    case BUILDING_ARMYCAMP: return { BARRACKS_COST_WOOD, BARRACKS_COST_FOOD, BARRACKS_COST_STONE, BARRACKS_COST_GOLD };
    // 【之后管】: TODO: 补全其他建筑的资源消耗
    default: return {0, 0, 0, 0};
    }
}

I32 Development::buildingActionTimeCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return 0;
    if (actNum == BUILDING_CENTER_CREATEFARMER) return VILLAGER_TRAIN_TIME;
    if (actNum == BUILDING_ARMYCAMP_CREATE_CLUBMAN) return CLUBMAN_TRAIN_TIME;
    if (actNum == BUILDING_ARMYCAMP_CREATE_SLINGER) return SLINGER_TRAIN_TIME;
    if (actNum == BUILDING_RANGE_CREATE_BOWMAN) return BOWMAN_TRAIN_TIME;
    if (actNum == BUILDING_STABLE_CREATE_SCOUT) return SCOUT_TRAIN_TIME;
    if (actNum == BUILDING_STOCK_UPGRADE_USETOOL) return TOOLWORKING_RESEARCH_TIME;
    if (actNum == BUILDING_CENTER_UPGRADE_TOOLAGE) return TOOL_AGE_RESEARCH_TIME;
    if (actNum == BUILDING_CENTER_UPGRADE_BRONZEAGE) return BRONZE_AGE_RESEARCH_TIME;
    if (actNum == BUILDING_CENTER_UPGRADE_IRONAGE) return IRON_AGE_RESEARCH_TIME;
    // 【之后管】: TODO: 补全其他动作的耗时
    return 0;
}

std::array<I32,4> Development::buildingActionResourceCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) return {0, 0, 0, 0};
    if (actNum == BUILDING_CENTER_CREATEFARMER) return { VILLAGER_COST_WOOD, VILLAGER_COST_FOOD, VILLAGER_COST_STONE, VILLAGER_COST_GOLD };
    if (actNum == BUILDING_ARMYCAMP_CREATE_CLUBMAN) return { CLUBMAN_COST_WOOD, CLUBMAN_COST_FOOD, CLUBMAN_COST_STONE, CLUBMAN_COST_GOLD };
    if (actNum == BUILDING_RANGE_CREATE_BOWMAN) return { BOWMAN_COST_WOOD, BOWMAN_COST_FOOD, BOWMAN_COST_STONE, BOWMAN_COST_GOLD };
    if (actNum == BUILDING_STOCK_UPGRADE_USETOOL) return { TOOLWORKING_COST_WOOD, TOOLWORKING_COST_FOOD, TOOLWORKING_COST_STONE, TOOLWORKING_COST_GOLD };
    if (actNum == BUILDING_CENTER_UPGRADE_TOOLAGE) return { TOOL_AGE_COST_WOOD, TOOL_AGE_COST_FOOD, TOOL_AGE_COST_STONE, TOOL_AGE_COST_GOLD };
    if (actNum == BUILDING_CENTER_UPGRADE_BRONZEAGE) return { BRONZE_AGE_COST_WOOD, BRONZE_AGE_COST_FOOD, BRONZE_AGE_COST_STONE, BRONZE_AGE_COST_GOLD };
    if (actNum == BUILDING_CENTER_UPGRADE_IRONAGE) return { IRON_AGE_COST_WOOD, IRON_AGE_COST_FOOD, IRON_AGE_COST_STONE, IRON_AGE_COST_GOLD };
    // 【之后管】: TODO: 补全其他动作的资源消耗
    return {0, 0, 0, 0};
}

void Development::notifyBuildingActionFinish(WeakPtr<Object> object, I32 action)
{
    (void)object;
    markTechnologyFinish(action);
}

void Development::tick() {}