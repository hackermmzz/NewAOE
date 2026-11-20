#include "Player.h"
#include "Server/core/Public/config/ObjectBaseValue.h"
/*
namespace{
// 产人类的建筑动作：完成的结果是凭空多出一个单位，跟科技无关。
// 新单位在创建时已经拿过当时的科技，不需要为了它去重算全场。
// 其余动作（研究/升级/升时代）都会改动科技的加成，属于需要重算的范畴。
bool isUnitCreateAction(I32 action){
    switch(action){
    case BUILDING_CENTER_CREATEFARMER:
    //兵营
    case BUILDING_ARMYCAMP_CREATE_CLUBMAN:
    case BUILDING_ARMYCAMP_CREATE_SHORTSWORD:
    case BUILDING_ARMYCAMP_CREATE_SLINGER:
    //靶场
    case BUILDING_RANGE_CREATE_BOWMAN:
    case BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN:
    case BUILDING_RANGE_CREATE_CHARIOT_ARCHER:
    case BUILDING_RANGE_CREATE_HORSE_ARCHER:
    case BUILDING_RANGE_CREATE_ELEPHANT_ARCHER:
    //马厩
    case BUILDING_STABLE_CREATE_SCOUT:
    case BUILDING_STABLE_CREATE_CAVALRY:
    case BUILDING_STABLE_CREATE_CHARIOT:
    case BUILDING_STABLE_CREATE_WAR_ELEPHANT:
    case BUILDING_STABLE_CREATE_CAMEL:
    //船坞
    case BUILDING_DOCK_CREATE_SAILING:
    case BUILDING_DOCK_CREATE_TRADEBOAT:
    case BUILDING_DOCK_CREATE_TRANSPORT:
    case BUILDING_DOCK_CREATE_SCOUTSHIP:
    case BUILDING_DOCK_CREATE_CATAPULT_TRIREME:
    case BUILDING_DOCK_CREATE_FIREGALLEY:
    //攻城工坊
    case BUILDING_SIEGE_CREATE_STONE_THROWER:
    case BUILDING_SIEGE_CREATE_BALLISTA:
    //学院 / 神殿
    case BUILDING_COLLAGE_CREATE_HOPLITE:
    case BUILDING_TEMPLE_CREATE_PRIEST:
        return true;
    default:
        return false;
    }
}

// 一条升级规则：某个建筑动作完成后，把 fromType 的对象整体变成 toType。
// fromType 传 -1 表示这是一条链的起点（只是解锁，场上没有前一级对象）。
// upgradeExisting 区分「已有对象跟着升级」（兵种、箭塔）和「只影响以后新建的」
// （城墙：研究完只解锁更高级城墙，已经砌好的城墙不变）。
struct UpgradeRule{
    I32  action;            //触发升级的建筑动作
    I32  sort;              //对象大类
    I32  fromType;          //升级前的具体类型，-1 表示链首
    I32  toType;            //升级后的具体类型
    bool upgradeExisting;   //场上已有的前一级对象是否一起升级
};

// 升级链取自 Public/var 下各枚举与策划给的升级顺序。
constexpr UpgradeRule kUpgradeRules[] = {
    //---- 兵营 ----
    //棍棒兵 → 刀斧兵
    {BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,      SORT_ARMY, AT_CLUBMAN,          AT_AXEMAN,          true},
    //短剑兵 → 阔剑兵 → 长剑兵 → 军团步兵
    {BUILDING_ARMYCAMP_UPGRADE_BROADSWORD,   SORT_ARMY, AT_SHORT_SWORDSMAN,  AT_BROAD_SWORDSMAN, true},
    {BUILDING_ARMYCAMP_UPGRADE_LONGSWORD,    SORT_ARMY, AT_BROAD_SWORDSMAN,  AT_LONG_SWORDSMAN,  true},
    {BUILDING_ARMYCAMP_UPGRADE_LEGION,       SORT_ARMY, AT_LONG_SWORDSMAN,   AT_LEGION,          true},
    //---- 靶场 ----
    //强弓兵 → 复合弓兵；弓骑兵 → 重装弓骑兵
    {BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,   SORT_ARMY, AT_IMPROVED_BOWMAN,  AT_COMPOSITE_BOWMAN,true},
    {BUILDING_RANGE_UPGRADE_HORSE_ARCHER,    SORT_ARMY, AT_HORSE_ARCHER,     AT_HEAVY_HORSE_ARCHER,true},
    //---- 马厩 ----
    //骑兵 → 重骑兵 → 铁甲骑兵；战车 → 刀轮战车；战象 → 装甲战象
    {BUILDING_STABLE_UPGRADE_CAVALRY,        SORT_ARMY, AT_CAVALRY,          AT_HEAVY_CAVALRY,   true},
    {BUILDING_STABLE_UPGRADE_CATAPHRACT,     SORT_ARMY, AT_HEAVY_CAVALRY,    AT_CATAPHRACT,      true},
    {BUILDING_STABLE_UPGRADE_CHARIOT,        SORT_ARMY, AT_CHARIOT,          AT_SCYTHECHARIOT,   true},
    {BUILDING_STABLE_UPGRADE_ELEPHANT,       SORT_ARMY, AT_WAR_ELEPHANT,     AT_ARMORED_ELEPHANT,true},
    //---- 攻城武器厂 ----
    //投石车 → 中型投石车 → 重型投石车；弩炮 → 重型弩炮
    {BUILDING_SIEGE_UPGRADE_CATAPULT,        SORT_ARMY, AT_STONE_THROWER,    AT_CATAPULT,        true},
    {BUILDING_SIEGE_UPGRADE_HEAVYCATAPULT,   SORT_ARMY, AT_CATAPULT,         AT_HEAVY_CATAPULT,  true},
    {BUILDING_SIEGE_UPGRADE_HELEPOLIS,       SORT_ARMY, AT_BALLISTA,         AT_HELEPOLIS,       true},
    //---- 船坞 ----
    //侦察船 → 桨帆船 → 三列桨帆船
    {BUILDING_DOCK_UPGRADE_WAR_GALLEY,       SORT_ARMY, AT_SCOUT_SHIP,       AT_WAR_GALLEY,      true},
    {BUILDING_DOCK_UPGRADE_TRIREME,          SORT_ARMY, AT_WAR_GALLEY,       AT_TRIREME,         true},
    //渔船 → 大型渔船；商船 → 大型商船；运输船 → 重型运输船
    {BUILDING_DOCK_UPGRADE_SAILING,          SORT_FARMER, FARMERTYPE_SMALL_SAILING,     FARMERTYPE_BIG_SAILING,     true},
    {BUILDING_DOCK_UPGRADE_TRADEBOAT,        SORT_FARMER, FARMERTYPE_SMALL_TRADE_BOAT,  FARMERTYPE_BIG_TRADE_BOAT,  true},
    {BUILDING_DOCK_UPGRADE_TRANSPORT,        SORT_FARMER, FARMERTYPE_LIGHT_TRANSPORT,  FARMERTYPE_HEAVY_TRANSPORT, true},
    //---- 学院 ----
    //方阵兵 → 十夫长 → 百夫长
    {BUILDING_COLLAGE_UPGRADE_PHALANX,       SORT_ARMY, AT_HOPLITE,          AT_PHALANX,         true},
    {BUILDING_COLLAGE_UPGRADE_CENTURION,     SORT_ARMY, AT_PHALANX,          AT_CENTURION,       true},
    //---- 城墙：只解锁更高级的城墙，已经砌好的不跟着变 ----
    {BUILDING_GRANARY_WALL,                  SORT_BUILDING, -1,                  BUILDING_SMALL_WALL,    false},
    {BUILDING_GRANARY_WALL_UPDATE_1,         SORT_BUILDING, BUILDING_SMALL_WALL, BUILDING_MEDIUM_WALL,   false},
    {BUILDING_GRANARY_WALL_UPDATE_2,         SORT_BUILDING, BUILDING_MEDIUM_WALL,BUILDING_FORTIFIED_WALL,false},
    //---- 箭塔：瞭望 → 警戒 → 防御 → 防御炮塔 ----
    {BUILDING_GRANARY_ARROWTOWER,            SORT_BUILDING, -1,                  BUILDING_WATCH_TOWER,   false},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_1,  SORT_BUILDING, BUILDING_WATCH_TOWER,  BUILDING_SENTRY_TOWER,  true},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_2,  SORT_BUILDING, BUILDING_SENTRY_TOWER, BUILDING_GUARD_TOWER,   true},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_3,  SORT_BUILDING, BUILDING_GUARD_TOWER,  BUILDING_BALLISTA_TOWER,true},
};

//按动作查升级规则，没有就返回 nullptr
const UpgradeRule* findUpgradeRule(I32 action){
    for(const UpgradeRule& rule : kUpgradeRules){
        if(rule.action == action){
            return &rule;
        }
    }
    return nullptr;
}

//查「某个大类下，从 fromType 往上一级」的规则
const UpgradeRule* findUpgradeRuleByFrom(I32 sort,I32 fromType){
    for(const UpgradeRule& rule : kUpgradeRules){
        if(rule.sort == sort && rule.fromType == fromType){
            return &rule;
        }
    }
    return nullptr;
}

// 把已经被 destroy 掉（SafePtr 取出来是空）的对象从容器里摘干净，
// 避免容器里长期挂着空槽位。
template<class T>
void purgeDestroyed(vector<SafePtr<T>>& container){
    for(auto it = container.begin();it != container.end();){
        if(it->IsNull()){
            it = container.erase(it);
        }else{
            ++it;
        }
    }
}

//按 globalID 从容器里摘掉一个对象。
//必须先判 IsNull：对象被 destroy 之后 SafePtr 的 operator-> 返回 nullptr，
//直接解引用会崩。内核完全可能「先把对象标死、再调 removeXXX」。
template<class T>
bool removeByGlobalNum(vector<SafePtr<T>>& container, I32 globalID){
    for(auto it = container.begin();it!=container.end();++it){
        if(it->IsNull()){
            continue;
        }
        if((*it)->getglobalNum() == globalID){
            container.erase(it);
            return true;
        }
    }
    return false;
}
}
*/
/*enum BuildingAction
{
    // ---------- 市镇中心 Town Center ----------
    BUILDING_CENTER_CREATEFARMER = 1,          // 创造村民
    BUILDING_CENTER_UPGRADE_TOOLAGE,           // 升级到工具时代（石器）
    BUILDING_CENTER_UPGRADE_BRONZEAGE,         // 升级到铜器时代（工具）
    BUILDING_CENTER_UPGRADE_IRONAGE,           // 升级到铁器时代（铜器）
    //BUILDING_CENTER_CITY_WATCH,                // 城防：全部建筑视野+3（青铜）

    // ---------- 房屋 House：无行为 ----------

    // ---------- 粮仓 Granary ----------
    BUILDING_GRANARY_WALL,                     // 研发初级城墙（工具）
    BUILDING_GRANARY_WALL_UPDATE_1,            // 升级为中级城墙（青铜）
    BUILDING_GRANARY_WALL_UPDATE_2,            // 升级为高级城墙（铁器）

    BUILDING_GRANARY_ARROWTOWER,               // 研发瞭望箭塔（工具）
    BUILDING_GRANARY_ARROWTOWER_UPGRADE_1,     // 升级为警戒箭塔（青铜）
    BUILDING_GRANARY_ARROWTOWER_UPGRADE_2,     // 升级为防御箭塔（铁器）
    BUILDING_GRANARY_ARROWTOWER_UPGRADE_3,     // 升级为防御炮塔（铁器，需弹道学）

    // ---------- 市场 Market ----------
    BUILDING_MARKET_WOOD_UPGRADE,              // 木工：伐木+20%、携带+2、射程+1（工具）
    BUILDING_MARKET_ARTISANSHIP,               // 工匠技艺：射程+1（青铜）
    BUILDING_MARKET_CRAFT_UPGRADE,             // 工艺：射程+1（铁器）

    BUILDING_MARKET_STONE_UPGRADE,             // 采石：采石+30%、投石兵攻/程+1（工具）
    BUILDING_MARKET_SIEGECRAFT,                // 攻城术：采石效率、村民拆墙塔（青铜）

    BUILDING_MARKET_GOLD_UPGRADE,              // 采金：采金+30%、携带+3（工具）
    BUILDING_MARKET_COINAGE,                   // 铸币：金矿+25%、贡品免税（工具）

    BUILDING_MARKET_FARM_UPGRADE,              // 驯养：农田+75（工具）
    BUILDING_MARKET_PLOW_UPGRADE,              // 犁：农田+100（青铜）
    BUILDING_MARKET_IRRIGATION,                // 灌溉：农田+125（铁器）

    BUILDING_MARKET_WHEEL_UPGRADE,             // 车轮：村民移速+50%（青铜）

    // ---------- 仓库 STOCK ----------
    BUILDING_STOCK_UPGRADE_USETOOL,            // 工具锻造：近战攻击+2（工具）
    BUILDING_STOCK_UPGRADE_METALWORKING,       // 金属加工：近战攻击+2（青铜）
    BUILDING_STOCK_UPGRADE_METALLURGY,         // 冶金：近战攻击+3（铁器）

    BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_1,   // 皮革护甲（步兵）：护甲+2（工具）
    BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_2, // 鳞甲（步兵）：护甲+2（青铜）
    BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_3, // 锁子甲（步兵）：护甲+2（铁器）

    BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_1,     // 皮革护甲（弓兵）：护甲+2（工具）
    BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_2,   // 鳞甲（弓兵）：护甲+2（青铜）
    BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_3,   // 锁子甲（弓兵）：护甲+2（铁器，解锁重装马弓手）

    BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_1,      // 皮革护甲（骑兵）：护甲+2（工具）
    BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_2,    // 鳞甲（骑兵）：护甲+2（青铜）
    BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_3,    // 锁子甲（骑兵）：护甲+2（铁器）

    BUILDING_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY, // 青铜盾：穿刺护甲+1（青铜）
    BUILDING_STOCK_UPGRADE_IRON_SHIELD,        // 铁盾：穿刺护甲+1（铁器）
    BUILDING_STOCK_UPGRADE_TOWER_SHIELD,       // 塔盾：穿刺护甲+1（铁器）

    // ---------- 兵营 Barracks ----------
    BUILDING_ARMYCAMP_CREATE_CLUBMAN,          // 训练棍棒兵（石器）
    BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,         // 升级为斧兵（石器）

    BUILDING_ARMYCAMP_CREATE_SHORTSWORD,       // 训练短剑兵（青铜）
    BUILDING_ARMYCAMP_UPGRADE_BROADSWORD,      // 升级为阔剑兵（青铜）
    BUILDING_ARMYCAMP_UPGRADE_LONGSWORD,       // 升级为长剑兵（青铜）
    BUILDING_ARMYCAMP_UPGRADE_LEGION,          // 升级为军团（铁器，需狂热）

    BUILDING_ARMYCAMP_CREATE_SLINGER,          // 训练投石兵（石器）

    // ---------- 靶场 Archery Range ----------
    BUILDING_RANGE_CREATE_BOWMAN,              // 训练弓手（工具）

    BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN,    // 训练强弓手（青铜）
    BUILDING_RANGE_UPGRADE_IMPROVEDBOW,        // 升级改良弓手（青铜）
    BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,      // 升级为复合弓（青铜）

    BUILDING_RANGE_CREATE_CHARIOT_ARCHER,      // 训练战车弓手（青铜，需车轮）

    BUILDING_RANGE_CREATE_HORSE_ARCHER,        // 训练马弓手（铁器）
    BUILDING_RANGE_UPGRADE_HORSE_ARCHER,       // 升级为重装马弓手（铁器，需锁子甲弓兵）

    BUILDING_RANGE_CREATE_ELEPHANT_ARCHER,     // 训练象弓手（铁器）

    // ---------- 马厩 Stable ----------
    BUILDING_STABLE_CREATE_SCOUT,              // 训练侦察骑兵（工具）

    BUILDING_STABLE_CREATE_CAVALRY,            // 训练骑兵（青铜）
    BUILDING_STABLE_UPGRADE_CAVALRY,           // 升级为重装骑兵（铁器）
    BUILDING_STABLE_UPGRADE_CATAPHRACT,        // 升级为铁甲骑兵（铁器，需冶金）

    BUILDING_STABLE_CREATE_CHARIOT,            // 训练驷马战车（青铜，需车轮）
    BUILDING_STABLE_UPGRADE_CHARIOT,           // 升级为刀轮战车（铁器，需贵族）

    BUILDING_STABLE_CREATE_WAR_ELEPHANT,       // 训练战象（铁器）
    BUILDING_STABLE_UPGRADE_ELEPHANT,          // 升级为装甲战象（铁器，需铁盾）

    BUILDING_STABLE_CREATE_CAMEL,              // 训练骆驼骑兵（青铜）

    // ---------- 船坞 Dock ----------
    BUILDING_DOCK_CREATE_SAILING,              // 造渔船（石器）
    BUILDING_DOCK_UPGRADE_SAILING,             // 升级为大型渔船（船）（青铜）

    BUILDING_DOCK_CREATE_TRADEBOAT,            // 造商船（石器）
    BUILDING_DOCK_UPGRADE_TRADEBOAT,           // 升级为商船（船）（青铜）

    BUILDING_DOCK_CREATE_TRANSPORT,            // 造运输船（工具）
    BUILDING_DOCK_UPGRADE_TRANSPORT,           // 升级为重型运输船（铁器）

    BUILDING_DOCK_CREATE_SCOUTSHIP,            // 造侦察船（工具）
    BUILDING_DOCK_UPGRADE_WAR_GALLEY,           // 升级为桨帆船（青铜）
    BUILDING_DOCK_UPGRADE_TRIREME,             // 升级为三列桨帆船（铁器）

    BUILDING_DOCK_CREATE_CATAPULT_TRIREME,     // 造无敌战船（铁器）

    BUILDING_DOCK_CREATE_FIREGALLEY,           // 造火帆船（铁器）

    // ---------- 攻城工坊 Siege Workshop ----------
    BUILDING_SIEGE_CREATE_STONE_THROWER,       // 造投石车（青铜）
    BUILDING_SIEGE_UPGRADE_CATAPULT,           // 升级为中型投石车（铁器）
    BUILDING_SIEGE_UPGRADE_HEAVYCATAPULT,      // 升级为重型投石车（铁器，需攻城术）

    BUILDING_SIEGE_CREATE_BALLISTA,            // 造弩炮（铁器）
    BUILDING_SIEGE_UPGRADE_HELEPOLIS,          // 升级为重弩炮（铁器，需工艺）

    // ---------- 学院 Academy ----------
    BUILDING_COLLAGE_CREATE_HOPLITE,           // 训练方阵兵（青铜）
    BUILDING_COLLAGE_UPGRADE_PHALANX,          // 升级为十夫长（铁器）
    BUILDING_COLLAGE_UPGRADE_CENTURION,        // 升级为百夫长（铁器，需贵族政治）

    // ---------- 政治机关 Government Center ----------
    BUILDING_GOVERNMENT_ARISTOCRACY,           // 贵族政治：学院单位移速+0.225（铁器）
    BUILDING_GOVERNMENT_NOBILITY,              // 贵族：骑兵（除象）生命+15%（铁器）
    BUILDING_GOVERNMENT_ENGINEERING,           // 工程学：攻城单位射程+2（铁器）
    BUILDING_GOVERNMENT_ALCHEMY,               // 炼金术：远程攻击+1、火焰船+6（铁器）
    BUILDING_GOVERNMENT_BALLISTICS,            // 弹道学：远程/攻城命中移动目标（铁器）
    BUILDING_GOVERNMENT_ARCHITECTURE,          // 建筑学：建筑生命+20%、建造-33%（铁器）
    BUILDING_GOVERNMENT_LOGISTICS,             // 后勤：兵营单位人口减半（铁器）

    // ---------- 神殿 Temple ----------、
    BUILDING_TEMPLE_CREATE_PRIEST,             // 训练祭司（青铜）
    BUILDING_TEMPLE_POLYTHEISM,                // 多神教：祭司移速+40%（工具）
    BUILDING_TEMPLE_MYSTICISM,                 // 神秘主义：祭司生命+20（青铜）
    BUILDING_TEMPLE_ASTROLOGY,                 // 占星术：转化与治疗效率+30%（青铜）
    BUILDING_TEMPLE_MEDICINE,                  // 医学：治疗速度+100%（青铜）
    BUILDING_TEMPLE_MONOTHEISM,                // 一神教：可转化建筑与其他祭司（铁器）
    BUILDING_TEMPLE_AFTERLIFE,                 // 来生：转化射程与视野+3（铁器）
    BUILDING_TEMPLE_FANATICISM,                // 狂热：祭司复活速度+50%、解锁军团（铁器）
    BUILDING_TEMPLE_ZEALOTRY,                  // 圣战：村民攻击+7、生命+40（铁器）
    //BUILDING_TEMPLE_THEOCRACY,                 // 神权政治：一群祭司转化后只需一人休息（铁器）
    BUILDING_TEMPLE_SACRIFICE,                 // 献祭：牺牲祭司立即转化敌方单位（铁器）
};*/
bool __BuildingActionIsCreatHuman(I32 BuildingAction){
    switch(BuildingAction){
    case BUILDING_CENTER_CREATEFARMER:
    case BUILDING_ARMYCAMP_CREATE_CLUBMAN:
    case BUILDING_ARMYCAMP_CREATE_SHORTSWORD:
    case BUILDING_ARMYCAMP_CREATE_SLINGER:
    case BUILDING_RANGE_CREATE_BOWMAN:
    case BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN:
    case BUILDING_RANGE_CREATE_CHARIOT_ARCHER:
    case BUILDING_RANGE_CREATE_HORSE_ARCHER:
    case BUILDING_RANGE_CREATE_ELEPHANT_ARCHER:
    case BUILDING_STABLE_CREATE_SCOUT:
    case BUILDING_STABLE_CREATE_CAVALRY:
    case BUILDING_STABLE_CREATE_CHARIOT:
    case BUILDING_STABLE_CREATE_WAR_ELEPHANT:
    case BUILDING_STABLE_CREATE_CAMEL:
    case BUILDING_DOCK_CREATE_SAILING:
    case BUILDING_DOCK_CREATE_TRADEBOAT:
    case BUILDING_DOCK_CREATE_TRANSPORT:
    case BUILDING_DOCK_CREATE_SCOUTSHIP:
    case BUILDING_DOCK_CREATE_CATAPULT_TRIREME:
    case BUILDING_DOCK_CREATE_FIREGALLEY:
    case BUILDING_SIEGE_CREATE_STONE_THROWER:
    case BUILDING_SIEGE_CREATE_BALLISTA:
    case BUILDING_COLLAGE_CREATE_HOPLITE:
    case BUILDING_TEMPLE_CREATE_PRIEST:
        return true;
    default:
        return false;
    }
}
/*
bool __BuildingActionIsImproveObject(I32 BuildingAction){
    switch(BuildingAction){
    case BUILDING_CENTER_CREATEFARMER:
    case BUILDING_ARMYCAMP_CREATE_CLUBMAN:
    case BUILDING_ARMYCAMP_CREATE_SHORTSWORD:
    case BUILDING_ARMYCAMP_CREATE_SLINGER:
    case BUILDING_RANGE_CREATE_BOWMAN:
    case BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN:
    case BUILDING_RANGE_CREATE_CHARIOT_ARCHER:
    case BUILDING_RANGE_CREATE_HORSE_ARCHER:
    case BUILDING_RANGE_CREATE_ELEPHANT_ARCHER:
    case BUILDING_STABLE_CREATE_SCOUT:
    case BUILDING_STABLE_CREATE_CAVALRY:
    case BUILDING_STABLE_CREATE_CHARIOT:
    case BUILDING_STABLE_CREATE_WAR_ELEPHANT:
    case BUILDING_STABLE_CREATE_CAMEL:
    case BUILDING_DOCK_CREATE_SAILING:
    case BUILDING_DOCK_CREATE_TRADEBOAT:
    case BUILDING_DOCK_CREATE_TRANSPORT:
    case BUILDING_DOCK_CREATE_SCOUTSHIP:
    case BUILDING_DOCK_CREATE_CATAPULT_TRIREME:
    case BUILDING_DOCK_CREATE_FIREGALLEY:
    case BUILDING_SIEGE_CREATE_STONE_THROWER:
    case BUILDING_SIEGE_CREATE_BALLISTA:
    case BUILDING_COLLAGE_CREATE_HOPLITE:
    case BUILDING_TEMPLE_CREATE_PRIEST:
        return true;
    default:
        return false;
    }
}
bool __BuildingActionIsScience(I32 BuildingAction){

}
*/

Player::Player(I32 civilization, I32 teamID){
    this->civilization = civilization;
    this->teamID = teamID;
    //init() 之前就 newXXX 会把未初始化的 playerID 写进对象归属，先给个非法值
    this->playerID = -1;
    ResourceGroup = ObjectBaseValue::getStartResource(civilization);
}

void Player::init(I32 playerID){this->playerID = playerID;}
Army* Player::newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience){
    rememberPlayerScience(playerScience);
    //内核传的可能是底层兵种，实际按当前科技给到已解锁的最高级别
    //type = resolveCreateType(SORT_ARMY,type);
    SafePtr<Army> a(new Army(detailPos,redirectObjectType(SORT_ARMY,type),playerScience));
    a->setBelong(playerID,teamID);
    armies.push_back(a);
    return a.get();
}

Farmer* Player::newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type){
    rememberPlayerScience(playerScience);
    //type = resolveCreateType(SORT_FARMER,type);
    SafePtr<Farmer> f(new Farmer(detailPos,playerScience,redirectObjectType(SORT_FARMER,type)));
    farmers.push_back(f);
    f->setBelong(playerID,teamID);
    return f.get();
}

Building* Player::newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 Percent){
    rememberPlayerScience(playerScience);
    //type = resolveCreateType(SORT_BUILDING,type);
    SafePtr<Building> b(new Building(redirectObjectType(SORT_BUILDING,type),blockPos,playerScience,Percent));
    buildings.push_back(b);
    b->setBelong(playerID,teamID);
    return b.get();
}

BuildingResource* Player::newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience,I32 Percent){
    rememberPlayerScience(playerScience);
    SafePtr<BuildingResource> br(new BuildingResource(type,blockPos,playerScience,Percent));
    buildingResources.push_back(br);
    br->setBelong(playerID,teamID);
    return br.get();
}

Missile* Player::newMissile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack){
    rememberPlayerScience(playerScience);
    SafePtr<Missile> m(new Missile(type,attacker,attackee,playerScience,attack));
    missiles.push_back(m);
    m->setBelong(playerID,teamID);
    return m.get();
}

// 只认第一次拿到的 Development：玩家自己的科技由外部创建后传入，
// 后续 newXXX 传进来的应当还是同一个，重复写入没有意义。
void Player::rememberPlayerScience(WeakPtr<Development> science){if(playerScience.expired() && !science.expired())playerScience = science;}

WeakPtr<Development> Player::getPlayerScience() const{return playerScience;}

vector<SafePtr<Army>>& Player::getArmy(){return armies;}

vector<SafePtr<Farmer>>& Player::getFarmer(){return farmers;}

vector<SafePtr<Building>>& Player::getBuilding(){return buildings;}

vector<SafePtr<BuildingResource>>& Player::getBuildingResource(){return buildingResources;}

vector<SafePtr<Missile>>& Player::getMissile(){return missiles;}

void Player::removeArmy(I32 globalID){
    for(auto a = armies.begin();a!=armies.end();++a){
        if(a->IsNull())continue;
        if((*a)->getSN()==static_cast<U64>(globalID)){
            armies.erase(a);
            return;     //erase 之后迭代器失效，不能继续 ++a
        }
    }
}

void Player::removeFarmer(I32 globalID){
    for(auto a = farmers.begin();a!=farmers.end();++a){
        if(a->IsNull())continue;
        if((*a)->getSN()==static_cast<U64>(globalID)){
            farmers.erase(a);
            return;     //erase 之后迭代器失效，不能继续 ++a
        }
    }
}

void Player::removeBuilding(I32 globalID){
    for(auto a = buildings.begin();a!=buildings.end();++a){
        if(a->IsNull())continue;
        if((*a)->getSN()==static_cast<U64>(globalID)){
            buildings.erase(a);
            return;     //erase 之后迭代器失效，不能继续 ++a
        }
    }
}

void Player::removeBuildingResource(I32 globalID){
    for(auto a = buildingResources.begin();a!=buildingResources.end();++a){
        if(a->IsNull())continue;
        if((*a)->getSN()==static_cast<U64>(globalID)){
            buildingResources.erase(a);
            return;     //erase 之后迭代器失效，不能继续 ++a
        }
    }
}

void Player::removeMissile(I32 globalID){
    for(auto a = missiles.begin();a!=missiles.end();++a){
        if(a->IsNull())continue;
        if((*a)->getSN()==static_cast<U64>(globalID)){
            missiles.erase(a);
            return;     //erase 之后迭代器失效，不能继续 ++a
        }
    }
}

I32 Player::getResource(I32 resourceSort){
    if(resourceSort < 0 || resourceSort >= static_cast<I32>(ResourceGroup.size())){
        return 0;
    }
    return ResourceGroup[resourceSort];
}

array<I32,4> Player::getResource(){return ResourceGroup;}

bool Player::changeResource(I32 resourceSort, I32 changeAmount){
    //资源种类来自 ResourceSort（木材/食物/石头/黄金），越界一律拒绝，
    //避免把 ProductSort 之类的枚举误传进来直接越界写。
    if(resourceSort < 0 || resourceSort >= static_cast<I32>(ResourceGroup.size())){
        return false;
    }
    if(ResourceGroup[resourceSort] + changeAmount < 0){
        return false;
    }
    ResourceGroup[resourceSort] += changeAmount;
    return true;
}

bool Player::changeResource(I32 changeWoodAmount, I32 changeFoodAmount, I32 changeStoneAmount, I32 changeGoldAmount){
    if(ResourceGroup[RESOURCESORT_WOOD] + changeWoodAmount < 0 || ResourceGroup[RESOURCESORT_FOOD] + changeFoodAmount < 0 || ResourceGroup[RESOURCESORT_STONE] + changeStoneAmount < 0 || ResourceGroup[RESOURCESORT_GOLD] + changeGoldAmount < 0){
        return false;
    }
    ResourceGroup[RESOURCESORT_WOOD]  += changeWoodAmount;
    ResourceGroup[RESOURCESORT_FOOD]  += changeFoodAmount;
    ResourceGroup[RESOURCESORT_STONE] += changeStoneAmount;
    ResourceGroup[RESOURCESORT_GOLD]  += changeGoldAmount;
    return true;
}

bool Player::receiveArmy(SafePtr<Army> army){
    if(army == nullptr)return false;
    if(army->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    army->setBelong(playerID,teamID);
    // 转化后的单位科技跟随本玩家，不再拷贝一份锁定在原阵营转换时刻的科技。
    army->setPlayerScience(playerScience);
    armies.push_back(army);
    return true;
}
bool Player::receiveBuilding(SafePtr<Building> building){
    if(building == nullptr)return false;
    if(building->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    building->setBelong(playerID,teamID);
    building->setPlayerScience(playerScience);
    buildings.push_back(building);
    return true;
}
bool Player::receiveFarmer(SafePtr<Farmer> farmer){
    if(farmer == nullptr)return false;
    if(farmer->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    farmer->setBelong(playerID,teamID);
    farmer->setPlayerScience(playerScience);
    farmers.push_back(farmer);
    return true;
}
bool Player::receiveBuildingResource(SafePtr<BuildingResource> buildingResource){
    if(buildingResource == nullptr)return false;
    if(buildingResource->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    // 不能复用 receiveBuilding：资源型建筑不进入 buildings 容器。
    buildingResource->setBelong(playerID,teamID);
    buildingResource->setPlayerScience(playerScience);
    buildingResources.push_back(buildingResource);
    return true;
}

void Player::notifyBuildingActionFinish(WeakPtr<Object> object, I32 action){
    if(object.expired()||__BuildingActionIsCreatHuman(action)){return;}
    I32 Sort=-1,Code=-1,RealType=-1;
    switch(action){
    case BUILDING_CENTER_UPGRADE_TOOLAGE:           // 升级到工具时代（石器）
    case BUILDING_CENTER_UPGRADE_BRONZEAGE:         // 升级到铜器时代（工具）
    case BUILDING_CENTER_UPGRADE_IRONAGE:           // 升级到铁器时代（铜器）
    case BUILDING_MARKET_WOOD_UPGRADE:              // 木工：伐木+20%、携带+2、射程+1（工具）
    case BUILDING_MARKET_ARTISANSHIP:               // 工匠技艺：射程+1（青铜）
    case BUILDING_MARKET_CRAFT_UPGRADE:             // 工艺：射程+1（铁器）
    case BUILDING_MARKET_STONE_UPGRADE:             // 采石：采石+30%、投石兵攻/程+1（工具）
    case BUILDING_MARKET_SIEGECRAFT:                // 攻城术：采石效率、村民拆墙塔（青铜）
    case BUILDING_MARKET_GOLD_UPGRADE:              // 采金：采金+30%、携带+3（工具）
    case BUILDING_MARKET_COINAGE:                   // 铸币：金矿+25%、贡品免税（工具）
    case BUILDING_MARKET_FARM_UPGRADE:              // 驯养：农田+75（工具）
    case BUILDING_MARKET_PLOW_UPGRADE:              // 犁：农田+100（青铜）
    case BUILDING_MARKET_IRRIGATION:                // 灌溉：农田+125（铁器）
    case BUILDING_MARKET_WHEEL_UPGRADE:             // 车轮：村民移速+50%（青铜）
    case BUILDING_STOCK_UPGRADE_USETOOL:            // 工具锻造：近战攻击+2（工具）
    case BUILDING_STOCK_UPGRADE_METALWORKING:       // 金属加工：近战攻击+2（青铜）
    case BUILDING_STOCK_UPGRADE_METALLURGY:         // 冶金：近战攻击+3（铁器）
    case BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_1:   // 皮革护甲（步兵）：护甲+2（工具）
    case BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_2: // 鳞甲（步兵）：护甲+2（青铜）
    case BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_3: // 锁子甲（步兵）：护甲+2（铁器）
    case BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_1:     // 皮革护甲（弓兵）：护甲+2（工具）
    case BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_2:   // 鳞甲（弓兵）：护甲+2（青铜）
    case BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_3:   // 锁子甲（弓兵）：护甲+2（铁器，解锁重装马弓手）
    case BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_1:      // 皮革护甲（骑兵）：护甲+2（工具）
    case BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_2:    // 鳞甲（骑兵）：护甲+2（青铜）
    case BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_3:    // 锁子甲（骑兵）：护甲+2（铁器）
    case BUILDING_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY: // 青铜盾：穿刺护甲+1（青铜）
    case BUILDING_STOCK_UPGRADE_IRON_SHIELD:        // 铁盾：穿刺护甲+1（铁器）
    case BUILDING_STOCK_UPGRADE_TOWER_SHIELD:       // 塔盾：穿刺护甲+1（铁器）
    case BUILDING_GOVERNMENT_ARISTOCRACY:           // 贵族政治：学院单位移速+0.225（铁器）
    case BUILDING_GOVERNMENT_NOBILITY:              // 贵族：骑兵（除象）生命+15%（铁器）
    case BUILDING_GOVERNMENT_ENGINEERING:           // 工程学：攻城单位射程+2（铁器）
    case BUILDING_GOVERNMENT_ALCHEMY:               // 炼金术：远程攻击+1、火焰船+6（铁器）
    case BUILDING_GOVERNMENT_BALLISTICS:            // 弹道学：远程/攻城命中移动目标（铁器）
    case BUILDING_GOVERNMENT_ARCHITECTURE:          // 建筑学：建筑生命+20%、建造-33%（铁器）
    case BUILDING_GOVERNMENT_LOGISTICS:             // 后勤：兵营单位人口减半（铁器）
    case BUILDING_TEMPLE_POLYTHEISM:                // 多神教：祭司移速+40%（工具）
    case BUILDING_TEMPLE_MYSTICISM:                 // 神秘主义：祭司生命+20（青铜）
    case BUILDING_TEMPLE_ASTROLOGY:                 // 占星术：转化与治疗效率+30%（青铜）
    case BUILDING_TEMPLE_MEDICINE:                  // 医学：治疗速度+100%（青铜）
    case BUILDING_TEMPLE_MONOTHEISM:                // 一神教：可转化建筑与其他祭司（铁器）
    case BUILDING_TEMPLE_AFTERLIFE:                 // 来生：转化射程与视野+3（铁器）
    case BUILDING_TEMPLE_FANATICISM:                // 狂热：祭司复活速度+50%、解锁军团（铁器）
    case BUILDING_TEMPLE_ZEALOTRY:                  // 圣战：村民攻击+7、生命+40（铁器）
    case BUILDING_TEMPLE_SACRIFICE:                 // 献祭：牺牲祭司立即转化敌方单位（铁器）
    refreshAllObjectScience();
    return;

    case BUILDING_GRANARY_WALL_UPDATE_1:            // 升级为中级城墙（青铜）
    RealType=BUILDING_MEDIUM_WALL;
    Sort=SORT_BUILDING,Code=21;
    break;
    case BUILDING_GRANARY_WALL_UPDATE_2:            // 升级为高级城墙（铁器）
    RealType=BUILDING_FORTIFIED_WALL;
    Sort=SORT_BUILDING,Code=21;
    break;
    case BUILDING_GRANARY_ARROWTOWER_UPGRADE_1:     // 升级为警戒箭塔（青铜）
    RealType=BUILDING_SENTRY_TOWER;
    Sort=SORT_BUILDING,Code=20;
    break;
    case BUILDING_GRANARY_ARROWTOWER_UPGRADE_2:     // 升级为防御箭塔（铁器）
    RealType=BUILDING_GUARD_TOWER;
    Sort=SORT_BUILDING,Code=20;
    break;
    case BUILDING_GRANARY_ARROWTOWER_UPGRADE_3:     // 升级为防御炮塔（铁器，需弹道学）
    RealType=BUILDING_BALLISTA_TOWER;
    Sort=SORT_BUILDING,Code=20;
    break;
    case BUILDING_ARMYCAMP_UPGRADE_CLUBMAN:         // 升级为斧兵（石器）
    RealType=AT_AXEMAN;
    Sort=SORT_ARMY,Code=3;
    break;
    case BUILDING_ARMYCAMP_UPGRADE_BROADSWORD:      // 升级为阔剑兵（青铜）
    RealType=AT_BROAD_SWORDSMAN;
    Sort=SORT_ARMY,Code=4;
    break;
    case BUILDING_ARMYCAMP_UPGRADE_LONGSWORD:       // 升级为长剑兵（青铜）
    RealType=AT_LONG_SWORDSMAN;
    Sort=SORT_ARMY,Code=4;
    break;
    case BUILDING_ARMYCAMP_UPGRADE_LEGION:          // 升级为军团（铁器，需狂热）
    RealType=AT_LEGION;
    Sort=SORT_ARMY,Code=4;
    break;
    case BUILDING_RANGE_UPGRADE_COMPOSITE_BOW:      // 升级为复合弓（青铜）
    RealType=AT_COMPOSITE_BOWMAN;
    Sort=SORT_ARMY,Code=5;
    break;
    case BUILDING_RANGE_UPGRADE_HORSE_ARCHER:       // 升级为重装马弓手（铁器，需锁子甲弓兵）
    RealType=AT_HEAVY_HORSE_ARCHER;
    Sort=SORT_ARMY,Code=6;
    break;
    case BUILDING_STABLE_UPGRADE_CAVALRY:           // 升级为重装骑兵（铁器）
    RealType=AT_HEAVY_CAVALRY;
    Sort=SORT_ARMY,Code=7;
    break;
    case BUILDING_STABLE_UPGRADE_CATAPHRACT:        // 升级为铁甲骑兵（铁器，需冶金）
    RealType=AT_CATAPHRACT;
    Sort=SORT_ARMY,Code=7;
    break;
    case BUILDING_STABLE_UPGRADE_CHARIOT:           // 升级为刀轮战车（铁器，需贵族）
    RealType=AT_SCYTHECHARIOT;
    Sort=SORT_ARMY,Code=8;
    break;
    case BUILDING_STABLE_UPGRADE_ELEPHANT:          // 升级为装甲战象（铁器，需铁盾）
    RealType=AT_ARMORED_ELEPHANT;
    Sort=SORT_ARMY,Code=9;
    break;
    case BUILDING_DOCK_UPGRADE_SAILING:             // 升级为大型渔船（船）（青铜）
    RealType=FARMERTYPE_BIG_SAILING;
    Sort=SORT_FARMER,Code=0;
    break;
    case BUILDING_DOCK_UPGRADE_TRADEBOAT:           // 升级为大型商船（船）（青铜）
    RealType=FARMERTYPE_BIG_TRADE_BOAT;
    Sort=SORT_FARMER,Code=1;
    break;
    case BUILDING_DOCK_UPGRADE_TRANSPORT:           // 升级为重型运输船（铁器）
    RealType=FARMERTYPE_HEAVY_TRANSPORT;
    Sort=SORT_FARMER,Code=2;
    break;
    case BUILDING_DOCK_UPGRADE_WAR_GALLEY:          // 升级为桨帆船（青铜）
    RealType=AT_WAR_GALLEY;
    Sort=SORT_ARMY,Code=12;
    break;
    case BUILDING_DOCK_UPGRADE_TRIREME:             // 升级为三列桨帆船（铁器）
    RealType=AT_TRIREME;
    Sort=SORT_ARMY,Code=12;
    break;
    case BUILDING_SIEGE_UPGRADE_CATAPULT:           // 升级为中型投石车（铁器）
    RealType=AT_CATAPULT;
    Sort=SORT_ARMY,Code=10;
    break;
    case BUILDING_SIEGE_UPGRADE_HEAVYCATAPULT:      // 升级为重型投石车（铁器，需攻城术）
    RealType=AT_HEAVY_CATAPULT;
    Sort=SORT_ARMY,Code=10;
    break;
    case BUILDING_SIEGE_UPGRADE_HELEPOLIS:          // 升级为重弩炮（铁器，需工艺）
    RealType=AT_HELEPOLIS;
    Sort=SORT_ARMY,Code=11;
    break;
    case BUILDING_COLLAGE_UPGRADE_PHALANX:          // 升级为十夫长（铁器）
    RealType=AT_PHALANX;
    Sort=SORT_ARMY,Code=13;
    break;
    case BUILDING_COLLAGE_UPGRADE_CENTURION:        // 升级为百夫长（铁器，需贵族政治）
    RealType=AT_CENTURION;
    Sort=SORT_ARMY,Code=13;
    break;
    };
    if(Sort<0)return;
    __OBJECTREALLYTYPE__[Code]=RealType;
    changeObjectType(Sort,Code);
}


/*I32 ObjectCanImprove(I32 sort,I32 type){
        switch (sort)
        {
        case SORT_FARMER:
        switch (type)
        {
        case FARMERTYPE_SMALL_SAILING:
            return 0;
        case FARMERTYPE_SMALL_TRADE_BOAT:
            return 1;
        case  FARMERTYPE_LIGHT_TRANSPORT:
            return 2;
        default:
            return -1;
        }
        case SORT_ARMY:
        switch (type)
        {
        case AT_CLUBMAN:
            return 3;
        case AT_SHORT_SWORDSMAN:
        case AT_BROAD_SWORDSMAN:
        case AT_LONG_SWORDSMAN:
            return 4;
        case AT_IMPROVED_BOWMAN:
            return 5;
        case AT_HORSE_ARCHER:
            return 6;
        case AT_CAVALRY:
        case AT_HEAVY_CAVALRY:
            return 7;
        case AT_CHARIOT:
            return 8;
        case AT_WAR_ELEPHANT:
            return 9;
        case AT_STONE_THROWER:
        case AT_CATAPULT:
            return 10;
        case AT_BALLISTA:
            return 11;
        case AT_SCOUT_SHIP:
        case AT_WAR_GALLEY:
            return 12;
        case AT_HOPLITE:
        case AT_PHALANX:
            return 13;
        default:
            return -1;
        }
        case SORT_BUILDING:
        switch (type)
        {
        case BUILDING_WATCH_TOWER:
        case BUILDING_SENTRY_TOWER:
        case BUILDING_GUARD_TOWER:
            return 20;
        case BUILDING_SMALL_WALL:
        case BUILDING_MEDIUM_WALL:
            return 21;
        default:
            return -1;
        }
        default:
            return -1;
        }
    }*/
/*
void Player::upgradeObjectsForAction(I32 action){
    const UpgradeRule* rule = findUpgradeRule(action);
    if(rule == nullptr || rule->fromType < 0 || !rule->upgradeExisting){
        //没配升级链 / 是链首（纯解锁）/ 只影响以后新建的（城墙），场上没有要改的对象
        return;
    }
    switch(rule->sort){
    case SORT_ARMY:{
        for(auto& army : armies){
            if(!army.IsNull() && army->getType() == rule->fromType){
                army->changeType(rule->toType);
            }
        }
        break;
    }
    case SORT_FARMER:{
        for(auto& farmer : farmers){
            if(!farmer.IsNull() && farmer->getType() == rule->fromType){
                farmer->changeType(rule->toType);
            }
        }
        break;
    }
    case SORT_BUILDING:{
        for(auto& building : buildings){
            if(!building.IsNull() && building->getType() == rule->fromType){
                building->changeType(rule->toType);
            }
        }
        break;
    }
    default:
        break;
    }
}*/
/*
I32 Player::resolveCreateType(I32 sort,I32 type) const{
    I32 current = type;
    //沿着升级链一路往上找，直到下一级的解锁动作还没完成为止。
    //已经是高级别的对象（比如转化来的重骑兵）不会往回退，因为链上找不到比它更高的就先停。
    for(;;){
        const UpgradeRule* next = findUpgradeRuleByFrom(sort,current);
        if(next == nullptr){
            break;
        }
        if(finishedActions.find(next->action) == finishedActions.end()){
            break;
        }
        current = next->toType;
    }
    return current;
}*/

void Player::refreshAllObjectScience(){
    //投掷物和资源型建筑按建成时的科技定格，不随之后的研究变化，所以不刷新。
    for(auto& army : armies){
        if(!army.IsNull())army->refresh();
    }
    for(auto& farmer : farmers){
        if(!farmer.IsNull())farmer->refresh();
    }
    for(auto& building : buildings){
        if(!building.IsNull())building->refresh();
    }
}

void Player::tick(){
}
I32 Player::redirectObjectType(I32 sort,I32 type){
        I32 tmp=ObjectCanImprove(sort,type);
        if(tmp<0||tmp>=__OBJECTREALLYTYPE_SIZE__)return type;
        if(__OBJECTREALLYTYPE__[tmp]<=type)return type;
        return __OBJECTREALLYTYPE__[tmp];
}
void Player::changeObjectType(I32 sort,I32 code){
        switch(sort){
            case SORT_BUILDING:{
                for(SafePtr<Building>& tmp:buildings){
                    if(tmp.IsNull())continue;
                    I32 a=ObjectCanImprove(tmp->getSort(),tmp->getType());
                    if(code==a&&code>=0)tmp->changeType(redirectObjectType(tmp->getSort(),tmp->getType()));
                }
                return;
            }
            case SORT_ARMY:{
                for(SafePtr<Army>& tmp:armies){
                    if(tmp.IsNull())continue;
                    I32 a=ObjectCanImprove(tmp->getSort(),tmp->getType());
                    if(code==a&&code>=0)tmp->changeType(redirectObjectType(tmp->getSort(),tmp->getType()));
                }
                return;
            }
            case SORT_FARMER:{
                for(SafePtr<Farmer>& tmp:farmers){
                    if(tmp.IsNull())continue;
                    I32 a=ObjectCanImprove(tmp->getSort(),tmp->getType());
                    if(code==a&&code>=0)tmp->changeType(redirectObjectType(tmp->getSort(),tmp->getType()));
                }
                return;
            }
            default:return;
        }
}
I32 Player::ObjectCanImprove(I32 sort,I32 type){
        switch (sort)
        {
        case SORT_FARMER:
        switch (type)
        {
        case FARMERTYPE_SMALL_SAILING:
            return 0;
        case FARMERTYPE_SMALL_TRADE_BOAT:
            return 1;
        case  FARMERTYPE_LIGHT_TRANSPORT:
            return 2;
        default:
            return -1;
        }
        case SORT_ARMY:
        switch (type)
        {
        case AT_CLUBMAN:
            return 3;
        case AT_SHORT_SWORDSMAN:
        case AT_BROAD_SWORDSMAN:
        case AT_LONG_SWORDSMAN:
            return 4;
        case AT_IMPROVED_BOWMAN:
            return 5;
        case AT_HORSE_ARCHER:
            return 6;
        case AT_CAVALRY:
        case AT_HEAVY_CAVALRY:
            return 7;
        case AT_CHARIOT:
            return 8;
        case AT_WAR_ELEPHANT:
            return 9;
        case AT_STONE_THROWER:
        case AT_CATAPULT:
            return 10;
        case AT_BALLISTA:
            return 11;
        case AT_SCOUT_SHIP:
        case AT_WAR_GALLEY:
            return 12;
        case AT_HOPLITE:
        case AT_PHALANX:
            return 13;
        default:
            return -1;
        }
        case SORT_BUILDING:
        switch (type)
        {
        case BUILDING_WATCH_TOWER:
        case BUILDING_SENTRY_TOWER:
        case BUILDING_GUARD_TOWER:
            return 20;
        case BUILDING_SMALL_WALL:
        case BUILDING_MEDIUM_WALL:
            return 21;
        default:
            return -1;
        }
        default:
            return -1;
        }
    }