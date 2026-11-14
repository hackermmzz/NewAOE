#pragma once

/********** 建筑种类 **********/
enum BUILDING_TYPE {
    BUILDING_HOME = 0,          //房屋
    BUILDING_GRANARY,           //谷仓
    BUILDING_CENTER,            //市镇中心
    BUILDING_STOCK,             //仓库
    BUILDING_FARM,              //农场
    BUILDING_MARKET,            //市场
    BUILDING_WATCH_TOWER,       //瞭望箭塔
    BUILDING_SENTRY_TOWER,      //警戒箭塔
    BUILDING_GUARD_TOWER,       //防御箭塔
    BUILDING_BALLISTA_TOWER,    //防御炮塔
    BUILDING_ARMYCAMP,          //军营
    BUILDING_STABLE,            //马厩
    BUILDING_RANGE,             //靶场
    BUILDING_DOCK,              //船坞
    BUILDING_SIEGE,             //攻城武器厂
    BUILDING_COLLAGE,           //学院
    BUILDING_GOVERNMENT_CENTER, //政府中心。这个还未增加，本次也不用增加。为枚举保留
    BUILDING_WONDER,            //世界奇观。这个还未增加，本次也不用增加。为枚举保留
    BUILDING_TEMPLE,            //神庙。这个还未增加，本次也不用增加
    BUILDING_SMALL_WALL,        //初级城墙。这个还未增加，本次也不用增加
    BUILDING_MEDIUM_WALL,       //中级城墙。这个还未增加，本次也不用增加。为枚举保留
    BUILDING_FORTIFIED_WALL,    //高级城墙。这个还未增加，本次也不用增加。为枚举保留
    BUILDING_TYPE_MAXNUM,
};


/********** 建筑动作 **********/
enum BuildingAction{
    //市镇中心
    BUILDING_CENTER_CREATEFARMER=1,         //生产村民
    BUILDING_CENTER_UPGRADE_TOOLAGE,        //升级到工具时代
    BUILDING_CENTER_UPGRADE_BRONZEAGE,      //升级到铜器时代
    BUILDING_CENTER_UPGRADE_IRONAGE,        //升级到铁器时代
    //谷仓
    BUILDING_GRANARY_WALL,                  //研发初级城墙
    BUILDING_GRANARY_WALL_UPDATE_1,         //升级为中级城墙
    BUILDING_GRANARY_WALL_UPDATE_2,         //升级为高级城墙（初级城墙->中级城墙->高级城墙）
    BUILDING_GRANARY_ARROWTOWER,            //研发瞭望箭塔
    BUILDING_GRANARY_ARROWTOWER_UPGRADE_1,  //升级为警戒箭塔
    BUILDING_GRANARY_ARROWTOWER_UPGRADE_2,  //升级为防御箭塔
    BUILDING_GRANARY_ARROWTOWER_UPGRADE_3,  //升级为防御炮塔（瞭望箭塔->警戒箭塔->防御箭塔->防御炮塔）
    //市场
    BUILDING_MARKET_WOOD_UPGRADE,
    BUILDING_MARKET_STONE_UPGRADE,
    BUILDING_MARKET_FARM_UPGRADE,
    BUILDING_MARKET_GOLD_UPGRADE,
    BUILDING_MARKET_WHEEL_UPGRADE,
    BUILDING_MARKET_CRAFT_UPGRADE,          // 研发工艺（铜器时代，木材加工升级版）
    BUILDING_MARKET_PLOW_UPGRADE,           // 研发犁（铜器时代，驯养动物升级版）

    //仓库
    BUILDING_STOCK_UPGRADE_USETOOL,
    BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY,
    BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER,
    BUILDING_STOCK_UPGRADE_DEFENSE_RIDER,
    BUILDING_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY,
    //军营
    BUILDING_ARMYCAMP_CREATE_CLUBMAN,
    BUILDING_ARMYCAMP_CREATE_SLINGER,
    BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,
    BUILDING_ARMYCAMP_CREATE_BROADSWORD,  // 训练阔剑兵（使用未使用的编号）
    BUILDING_ARMYCAMP_UPGRADE_BROADSWORD,  // 升级阔剑兵科技（使用未使用的编号）

    //靶场
    BUILDING_RANGE_CREATE_BOWMAN,
    BUILDING_RANGE_CREATE_CHARIOT_ARCHER,   // 训练战车弓箭手
    BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN,   // 训练复合弓兵
    BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,  // 升级复合弓科技

    //马厩
    BUILDING_STABLE_CREATE_SCOUT,
    BUILDING_STABLE_CREATE_CHARIOT,
    BUILDING_STABLE_CREATE_CAVALRY,

    //船坞
    BUILDING_DOCK_CREATE_SAILING,
    BUILDING_DOCK_CREATE_WOOD_BOAT,
    BUILDING_DOCK_CREATE_SHIP,
    //攻城武器厂
    BUILDING_SIEGE_CREATE_STONE_THROWER,
    //学院
    BUILDING_COLLAGE_CREATE_HOPLITE,
    // 兼容性追加：兵营后勤科技（避免改变已有建筑动作编号）
    BUILDING_ARMYCAMP_RESEARCH_LOGISTICS,
};
enum BuildingAction
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
};

/*******************************建筑地基****************************/
enum FOUNDATION {
    FOUNDATION_SMALL = 0,
    FOUNDATION_MIDDLE,
    FOUNDATION_BIG,
    FOUNDATION_HUGE
};
