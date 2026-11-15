#include "development.h"

#include "Server/core/Module/Player/Player.h"
#include "Server/core/BaseObject/Building.h"
#include "Server/core/BaseObject/BuildingResource.h"
#include "Server/core/Public/config/Core.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Farmer.h"

/*==========================================================================
 * 数值约定（跨平台一致性要求，务必遵守）
 *
 * 本模块所有比例一律用 Double（= Fixed<I64, I64, 32>）运算，代码里不允许
 * 出现 double / float 或者带小数点的字面量。
 *
 * 定点数的 + - * / 全程是整数移位与整数除法（除法走 128/64 的整数实现），
 * 不经过任何浮点路径，所以 x86 / ARM 等不同架构上结果完全一致。
 *
 * 比例统一写成两个整数构造的商：
 *     Double(3)  / Double(2)     // 1.5
 *     Double(23) / Double(20)    // 1.15
 *     (Double(100) + Double(p)) / Double(100)
 * 不要用 Fixed::FromDouble(1.5) —— 那个函数内部是真的 double 乘法。
 *========================================================================*/

/*==========================================================================
 * 数据来源：《帝国时代：决定版全面数据》
 *   - 科技总表         → 各科技的费用、研究时间、时代
 *   - 建筑数据         → 各建筑的费用、建造时间、前置建筑
 *   - 单位数据         → 各兵种的训练费用、训练时间
 *   - 文明加成与开局   → 各文明的缺失科技 / 缺失建筑
 * 表里没给到的项填了估算值并标 TODO(估)，不要当成真实数值用。
 *========================================================================*/

namespace {

// 时代门槛，直接用 NEW_CIVILIZATION_STAGE 的值（1=石器 2=工具 3=青铜 4=铁器）
constexpr I32 ST_STONE  = CIVILIZATION_STONEAGE;
constexpr I32 ST_TOOL   = CIVILIZATION_TOOLAGE;
constexpr I32 ST_BRONZE = CIVILIZATION_BRONZEAGE;
constexpr I32 ST_IRON   = CIVILIZATION_IRONAGE;

constexpr I32 STAGE_MIN = ST_STONE;
constexpr I32 STAGE_MAX = ST_IRON;

#define DEV_NONE ::TechnologyBits()
#define DEV_T(x) ::techBit(x)

/*---------------------------- 建筑动作规则 ----------------------------
 * 一条规则 = 一个 BuildingAction 的完整前置条件与代价。
 * 表按 action 编号顺序排列（action 从 1 开始），所以查表时直接
 * kActions[action - 1]，不需要遍历。
 *
 * stage         ：最低时代要求
 * requireTech   ：前置科技位图，判断方式 = 已完成的科技 & 前置科技 == 前置科技
 * requireBuilding：除动作所在建筑外，还需要已经建成的建筑类型；-1 表示不需要
 *                  （AoE 里研究类动作都在对应建筑内，绝大多数填 -1）
 * food/gold/wood/stone：资源消耗
 * seconds       ：研究 / 训练耗时（秒），运行时 × GAME_TPS 换成帧
 * createSort/createType：造人动作的产出；-1 表示这不是造人动作
 *--------------------------------------------------------------------*/
struct ActionRule
{
    I32 action;
    I32 stage;
    TechnologyBits requireTech;
    I32 requireBuilding;
    I32 food;
    I32 gold;
    I32 wood;
    I32 stone;
    I32 seconds;
    I32 createSort;
    I32 createType;
};

constexpr ActionRule kActions[] = {
    // -------------------- 市镇中心 --------------------
    /*  1 造农民        */ {BUILDING_CENTER_CREATEFARMER,        ST_STONE,  DEV_NONE, -1,  50,    0,    0,    0,  20, SORT_FARMER, FARMERTYPE_FARMER},
    /*  2 升工具时代    */ {BUILDING_CENTER_UPGRADE_TOOLAGE,     ST_STONE,  DEV_NONE, -1, 500,    0,    0,    0, 120, -1, -1},
    /*  3 升青铜时代    */ {BUILDING_CENTER_UPGRADE_BRONZEAGE,    ST_TOOL,   DEV_T(TECH_CENTER_UPGRADE_TOOLAGE),    -1, 800,    0,    0,    0, 140, -1, -1},
    /*  4 升铁器时代    */ {BUILDING_CENTER_UPGRADE_IRONAGE,      ST_BRONZE, DEV_T(TECH_CENTER_UPGRADE_BRONZEAGE),   -1, 1000, 800,    0,    0, 160, -1, -1},
    // -------------------- 粮仓：城墙与箭塔 --------------------
    /*  5 研发初级城墙  */ {BUILDING_GRANARY_WALL,                ST_TOOL,   DEV_NONE, -1,    0,    0,    0,    0,   0, -1, -1},
    /*  6 中级城墙      */ {BUILDING_GRANARY_WALL_UPDATE_1,       ST_BRONZE, DEV_T(TECH_GRANARY_WALL),                -1,    0,    0,    0,    0,   0, -1, -1},
    /*  7 高级城墙      */ {BUILDING_GRANARY_WALL_UPDATE_2,       ST_IRON,   DEV_T(TECH_GRANARY_WALL_UPDATE_1),       -1,    0,    0,    0,    0,   0, -1, -1},
    /*  8 研发瞭望箭塔  */ {BUILDING_GRANARY_ARROWTOWER,          ST_TOOL,   DEV_NONE, -1,    0,    0,    0,  150,   0, -1, -1},
    /*  9 警戒箭塔      */ {BUILDING_GRANARY_ARROWTOWER_UPGRADE_1, ST_BRONZE, DEV_T(TECH_GRANARY_ARROWTOWER),         -1,    0,    0,    0,  150,   0, -1, -1},
    /* 10 防御箭塔      */ {BUILDING_GRANARY_ARROWTOWER_UPGRADE_2, ST_IRON,  DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_1), -1,  0,    0,    0,  150,   0, -1, -1},
    /* 11 防御炮塔      */ {BUILDING_GRANARY_ARROWTOWER_UPGRADE_3, ST_IRON,  DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_2) | DEV_T(TECH_GOVERNMENT_BALLISTICS), -1, 0, 0, 0, 150, 0, -1, -1},
    // -------------------- 市场：经济 --------------------
    /* 12 木工          */ {BUILDING_MARKET_WOOD_UPGRADE,         ST_TOOL,   DEV_NONE, -1,  120,    0,   75,    0,  60, -1, -1},
    /* 13 工匠技艺      */ {BUILDING_MARKET_ARTISANSHIP,          ST_BRONZE, DEV_T(TECH_MARKET_WOOD_UPGRADE),         -1,  170,    0,  150,    0,  80, -1, -1},
    /* 14 工艺          */ {BUILDING_MARKET_CRAFT_UPGRADE,        ST_IRON,   DEV_T(TECH_MARKET_ARTISANSHIP),          -1,  240,    0,  200,    0, 100, -1, -1},
    /* 15 采石          */ {BUILDING_MARKET_STONE_UPGRADE,        ST_TOOL,   DEV_NONE, -1,  100,    0,    0,   50,  30, -1, -1},
    /* 16 攻城术        */ {BUILDING_MARKET_SIEGECRAFT,           ST_BRONZE, DEV_T(TECH_MARKET_STONE_UPGRADE),        -1,  190,    0,    0,  100,  60, -1, -1},
    /* 17 采金          */ {BUILDING_MARKET_GOLD_UPGRADE,         ST_TOOL,   DEV_NONE, -1,  120,    0,  100,    0,  50, -1, -1},
    /* 18 铸币          */ {BUILDING_MARKET_COINAGE,              ST_TOOL,   DEV_T(TECH_MARKET_GOLD_UPGRADE),         -1,  200,  100,    0,    0,  60, -1, -1},
    /* 19 驯养          */ {BUILDING_MARKET_FARM_UPGRADE,         ST_TOOL,   DEV_NONE, -1,  150,    0,   50,    0,  40, -1, -1},
    /* 20 犁            */ {BUILDING_MARKET_PLOW_UPGRADE,         ST_BRONZE, DEV_T(TECH_MARKET_FARM_UPGRADE),         -1,  250,    0,   75,    0,  75, -1, -1},
    /* 21 灌溉          */ {BUILDING_MARKET_IRRIGATION,           ST_IRON,   DEV_T(TECH_MARKET_PLOW_UPGRADE),         -1,  300,    0,  100,    0, 100, -1, -1},
    /* 22 车轮          */ {BUILDING_MARKET_WHEEL_UPGRADE,        ST_BRONZE, DEV_T(TECH_CENTER_UPGRADE_TOOLAGE),       -1,  175,    0,   75,    0,  75, -1, -1},
    // -------------------- 仓库：军事 --------------------
    /* 23 工具锻造      */ {BUILDING_STOCK_UPGRADE_USETOOL,           ST_TOOL,   DEV_NONE, -1,  100,    0,    0,    0,  40, -1, -1},
    /* 24 金属加工      */ {BUILDING_STOCK_UPGRADE_METALWORKING,      ST_BRONZE, DEV_T(TECH_STOCK_UPGRADE_USETOOL),      -1,  200,  120,    0,    0,  75, -1, -1},
    /* 25 冶金          */ {BUILDING_STOCK_UPGRADE_METALLURGY,        ST_IRON,   DEV_T(TECH_STOCK_UPGRADE_METALWORKING), -1,  300,  180,    0,    0, 100, -1, -1},
    /* 26 皮革护甲步兵  */ {BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_1, ST_TOOL,  DEV_NONE, -1,   75,    0,    0,    0,  30, -1, -1},
    /* 27 鳞甲步兵      */ {BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_2, ST_BRONZE, DEV_T(TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_1), -1, 100, 50, 0, 0, 60, -1, -1},
    /* 28 锁子甲步兵    */ {BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_3, ST_IRON,  DEV_T(TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_2), -1, 125, 100, 0, 0, 75, -1, -1},
    /* 29 皮革护甲弓兵  */ {BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_1,   ST_TOOL,  DEV_NONE, -1,  100,    0,    0,    0,  30, -1, -1},
    /* 30 鳞甲弓兵      */ {BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_2,   ST_BRONZE, DEV_T(TECH_STOCK_UPGRADE_DEFENSE_ARCHER_1), -1, 125, 50, 0, 0, 60, -1, -1},
    /* 31 锁子甲弓兵    */ {BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_3,   ST_IRON,  DEV_T(TECH_STOCK_UPGRADE_DEFENSE_ARCHER_2), -1, 150, 100, 0, 0, 75, -1, -1},
    /* 32 皮革护甲骑兵  */ {BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_1,    ST_TOOL,  DEV_NONE, -1,  125,    0,    0,    0,  30, -1, -1},
    /* 33 鳞甲骑兵      */ {BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_2,    ST_BRONZE, DEV_T(TECH_STOCK_UPGRADE_DEFENSE_RIDER_1), -1, 150, 50, 0, 0, 60, -1, -1},
    /* 34 锁子甲骑兵    */ {BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_3,    ST_IRON,  DEV_T(TECH_STOCK_UPGRADE_DEFENSE_RIDER_2), -1, 175, 100, 0, 0, 75, -1, -1},
    /* 35 青铜盾        */ {BUILDING_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY, ST_BRONZE, DEV_NONE, -1, 150, 180, 0, 0, 50, -1, -1},
    /* 36 铁盾          */ {BUILDING_STOCK_UPGRADE_IRON_SHIELD,       ST_IRON,   DEV_T(TECH_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY), -1, 200, 320, 0, 0, 75, -1, -1},
    /* 37 塔盾          */ {BUILDING_STOCK_UPGRADE_TOWER_SHIELD,      ST_IRON,   DEV_T(TECH_STOCK_UPGRADE_IRON_SHIELD),  -1,  250,  400,    0,    0, 100, -1, -1},
    // -------------------- 兵营：步兵 --------------------
    /* 38 造棍棒兵      */ {BUILDING_ARMYCAMP_CREATE_CLUBMAN,     ST_STONE,  DEV_NONE, -1,  50,    0,    0,    0,  27, SORT_ARMY, AT_CLUBMAN},
    /* 39 升斧兵        */ {BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,    ST_STONE,  DEV_NONE, -1,  100,   0,    0,    0,   0, -1, -1},
    /* 40 造短剑兵      */ {BUILDING_ARMYCAMP_CREATE_SHORTSWORD,  ST_BRONZE, DEV_NONE, -1,  35,   15,    0,    0,  27, SORT_ARMY, AT_SHORT_SWORDSMAN},
    /* 41 升阔剑兵      */ {BUILDING_ARMYCAMP_UPGRADE_BROADSWORD, ST_BRONZE, DEV_T(TECH_ARMYCAMP_UPGRADE_CLUBMAN),   -1,  140,   50,    0,    0,   0, -1, -1},
    /* 42 升长剑兵      */ {BUILDING_ARMYCAMP_UPGRADE_LONGSWORD,  ST_BRONZE, DEV_T(TECH_ARMYCAMP_UPGRADE_BROADSWORD),-1,  240,  100,    0,    0,   0, -1, -1},
    /* 43 升军团        */ {BUILDING_ARMYCAMP_UPGRADE_LEGION,     ST_IRON,   DEV_T(TECH_ARMYCAMP_UPGRADE_LONGSWORD) | DEV_T(TECH_TEMPLE_FANATICISM), -1, 1400, 600, 0, 0, 0, -1, -1},
    /* 44 造投石兵      */ {BUILDING_ARMYCAMP_CREATE_SLINGER,     ST_STONE,  DEV_NONE, -1,  40,    0,    0,   10,  30, SORT_ARMY, AT_SLINGER},
    // -------------------- 靶场：弓兵 --------------------
    /* 45 造弓手        */ {BUILDING_RANGE_CREATE_BOWMAN,             ST_TOOL,   DEV_NONE, -1,  40,    0,   20,    0,  30, SORT_ARMY, AT_BOWMAN},
    /* 46 造强弓手      */ {BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN,   ST_BRONZE, DEV_NONE, -1,  40,   20,    0,    0,  30, SORT_ARMY, AT_IMPROVED_BOWMAN},
    /* 47 升改良弓手    */ {BUILDING_RANGE_UPGRADE_IMPROVEDBOW,       ST_BRONZE, DEV_NONE, -1,  100,   50,    0,    0,   0, -1, -1}, // TODO(估) 费用
    /* 48 升复合弓      */ {BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,     ST_BRONZE, DEV_T(TECH_RANGE_UPGRADE_IMPROVEDBOW), -1, 150, 100, 0, 0, 0, -1, -1}, // TODO(估) 费用
    /* 49 造战车弓手    */ {BUILDING_RANGE_CREATE_CHARIOT_ARCHER,     ST_BRONZE, DEV_T(TECH_MARKET_WHEEL_UPGRADE), -1, 40, 0, 70, 0, 40, SORT_ARMY, AT_CHARIOT_ARCHER},
    /* 50 造马弓手      */ {BUILDING_RANGE_CREATE_HORSE_ARCHER,       ST_IRON,   DEV_NONE, -1,  50,   70,    0,    0,  40, SORT_ARMY, AT_HORSE_ARCHER},
    /* 51 升重装马弓手  */ {BUILDING_RANGE_UPGRADE_HORSE_ARCHER,      ST_IRON,   DEV_T(TECH_STOCK_UPGRADE_DEFENSE_ARCHER_3), -1, 200, 150, 0, 0, 0, -1, -1}, // TODO(估) 费用
    /* 52 造象弓手      */ {BUILDING_RANGE_CREATE_ELEPHANT_ARCHER,    ST_IRON,   DEV_NONE, -1, 180,   60,    0,    0,  50, SORT_ARMY, AT_ELEPHANT_ARCHER},
    // -------------------- 马厩：骑兵 --------------------
    /* 53 造侦察骑兵    */ {BUILDING_STABLE_CREATE_SCOUT,          ST_TOOL,   DEV_NONE, -1,   90,    0,    0,    0,  30, SORT_ARMY, AT_SCOUT},
    /* 54 造骑兵        */ {BUILDING_STABLE_CREATE_CAVALRY,        ST_BRONZE, DEV_NONE, -1,   70,   80,    0,    0,  40, SORT_ARMY, AT_CAVALRY},
    /* 55 升重骑兵      */ {BUILDING_STABLE_UPGRADE_CAVALRY,       ST_IRON,   DEV_NONE, -1,  250,  150,    0,    0,   0, -1, -1}, // TODO(估) 费用
    /* 56 升铁甲骑兵    */ {BUILDING_STABLE_UPGRADE_CATAPHRACT,    ST_IRON,   DEV_T(TECH_STOCK_UPGRADE_METALLURGY),     -1, 1600,  600,    0,    0,   0, -1, -1},
    /* 57 造驷马战车    */ {BUILDING_STABLE_CREATE_CHARIOT,        ST_BRONZE, DEV_T(TECH_MARKET_WHEEL_UPGRADE),        -1,   40,    0,   60,    0,  40, SORT_ARMY, AT_CHARIOT},
    /* 58 升刀轮战车    */ {BUILDING_STABLE_UPGRADE_CHARIOT,       ST_IRON,   DEV_T(TECH_GOVERNMENT_ARISTOCRACY),      -1,    0, 1000, 1400,    0, 200, -1, -1},
    /* 59 造战象        */ {BUILDING_STABLE_CREATE_WAR_ELEPHANT,   ST_IRON,   DEV_NONE, -1,  170,   40,    0,    0,  50, SORT_ARMY, AT_WAR_ELEPHANT},
    /* 60 升装甲战象    */ {BUILDING_STABLE_UPGRADE_ELEPHANT,      ST_IRON,   DEV_T(TECH_STOCK_UPGRADE_TOWER_SHIELD),   -1,  300,  200,    0,    0,   0, -1, -1}, // TODO(估) 费用
    /* 61 造骆驼骑兵    */ {BUILDING_STABLE_CREATE_CAMEL,          ST_BRONZE, DEV_NONE, -1,   70,   60,    0,    0,  40, SORT_ARMY, AT_CAMEL_RIDER},
    // -------------------- 船坞 --------------------
    /* 62 造渔船        */ {BUILDING_DOCK_CREATE_SAILING,          ST_STONE,  DEV_NONE, -1,    0,    0,   50,    0,  20, SORT_FARMER, FARMERTYPE_SMALL_SAILING},
    /* 63 升大型渔船    */ {BUILDING_DOCK_UPGRADE_SAILING,         ST_BRONZE, DEV_NONE, -1,  100,    0,   50,    0,   0, -1, -1}, // TODO(估) 费用
    /* 64 造商船        */ {BUILDING_DOCK_CREATE_TRADEBOAT,        ST_STONE,  DEV_NONE, -1,    0,    0,  100,    0,  25, SORT_FARMER, FARMERTYPE_SMALL_TRADE_BOAT},
    /* 65 升大型商船    */ {BUILDING_DOCK_UPGRADE_TRADEBOAT,       ST_BRONZE, DEV_NONE, -1,  150,    0,  100,    0,   0, -1, -1}, // TODO(估) 费用
    /* 66 造运输船      */ {BUILDING_DOCK_CREATE_TRANSPORT,        ST_TOOL,   DEV_NONE, -1,    0,    0,  150,    0,  38, SORT_FARMER, FARMERTYPE_LIGHT_TRANSPORT},
    /* 67 升重型运输船  */ {BUILDING_DOCK_UPGRADE_TRANSPORT,       ST_IRON,   DEV_NONE, -1,  200,    0,  150,    0,   0, -1, -1}, // TODO(估) 费用
    /* 68 造侦察船      */ {BUILDING_DOCK_CREATE_SCOUTSHIP,        ST_TOOL,   DEV_NONE, -1,    0,    0,  135,    0,  30, SORT_ARMY, AT_SCOUT_SHIP},
    /* 69 升桨帆船      */ {BUILDING_DOCK_UPGRADE_WAR_GALLEY,      ST_BRONZE, DEV_NONE, -1,  200,    0,  100,    0,   0, -1, -1}, // TODO(估) 费用
    /* 70 升三列桨帆船  */ {BUILDING_DOCK_UPGRADE_TRIREME,         ST_IRON,   DEV_T(TECH_DOCK_UPGRADE_WAR_GALLEY),    -1,  300,    0,  200,    0,   0, -1, -1}, // TODO(估) 费用
    /* 71 造无敌战船    */ {BUILDING_DOCK_CREATE_CATAPULT_TRIREME, ST_IRON,   DEV_T(TECH_DOCK_UPGRADE_TRIREME),        -1,    0,   75,  135,    0,  45, SORT_ARMY, AT_CATAPULT_TRIREME},
    /* 72 造火帆船      */ {BUILDING_DOCK_CREATE_FIREGALLEY,       ST_IRON,   DEV_NONE, -1,    0,   50,  100,    0,  45, SORT_ARMY, AT_FIRESAILBOAT}, // TODO(估) 费用
    // -------------------- 攻城工坊 --------------------
    /* 73 造投石车      */ {BUILDING_SIEGE_CREATE_STONE_THROWER,   ST_BRONZE, DEV_NONE, -1,    0,   80,  180,    0,  60, SORT_ARMY, AT_STONE_THROWER},
    /* 74 升中型投石车  */ {BUILDING_SIEGE_UPGRADE_CATAPULT,       ST_IRON,   DEV_NONE, -1,  300,  200,    0,    0,   0, -1, -1}, // TODO(估) 费用
    /* 75 升重型投石车  */ {BUILDING_SIEGE_UPGRADE_HEAVYCATAPULT,  ST_IRON,   DEV_T(TECH_MARKET_SIEGECRAFT),           -1,  400,  300,    0,    0,   0, -1, -1}, // TODO(估) 费用
    /* 76 造弩炮        */ {BUILDING_SIEGE_CREATE_BALLISTA,        ST_IRON,   DEV_NONE, -1,    0,   80,  100,    0,  50, SORT_ARMY, AT_BALLISTA},
    /* 77 升重弩炮      */ {BUILDING_SIEGE_UPGRADE_HELEPOLIS,      ST_IRON,   DEV_T(TECH_MARKET_CRAFT_UPGRADE),        -1, 1200,    0, 1000,    0,   0, -1, -1},
    // -------------------- 学院 --------------------
    /* 78 造方阵兵      */ {BUILDING_COLLAGE_CREATE_HOPLITE,       ST_BRONZE, DEV_NONE, -1,   60,   40,    0,    0,  36, SORT_ARMY, AT_HOPLITE},
    /* 79 升十夫长      */ {BUILDING_COLLAGE_UPGRADE_PHALANX,      ST_IRON,   DEV_NONE, -1,  300,  200,    0,    0,   0, -1, -1}, // TODO(估) 费用
    /* 80 升百夫长      */ {BUILDING_COLLAGE_UPGRADE_CENTURION,    ST_IRON,   DEV_T(TECH_GOVERNMENT_ARISTOCRACY),      -1,  400,  300,    0,    0,   0, -1, -1}, // TODO(估) 费用
    // -------------------- 政治机关 --------------------
    /* 81 贵族政治      */ {BUILDING_GOVERNMENT_ARISTOCRACY,       ST_IRON,   DEV_NONE, -1,  175,  150,    0,    0,  60, -1, -1},
    /* 82 贵族          */ {BUILDING_GOVERNMENT_NOBILITY,          ST_IRON,   DEV_NONE, -1,  175,  120,    0,    0,  70, -1, -1},
    /* 83 工程学        */ {BUILDING_GOVERNMENT_ENGINEERING,       ST_IRON,   DEV_T(TECH_MARKET_SIEGECRAFT),           -1,  200,    0,  100,    0,  70, -1, -1},
    /* 84 炼金术        */ {BUILDING_GOVERNMENT_ALCHEMY,           ST_IRON,   DEV_NONE, -1,  250,  200,    0,    0, 100, -1, -1},
    /* 85 弹道学        */ {BUILDING_GOVERNMENT_BALLISTICS,        ST_IRON,   DEV_T(TECH_GOVERNMENT_ENGINEERING),      -1,  200,   50,    0,    0,  60, -1, -1},
    /* 86 建筑学        */ {BUILDING_GOVERNMENT_ARCHITECTURE,      ST_IRON,   DEV_NONE, -1,  100,    0,    0,    0,  15, -1, -1},
    /* 87 后勤          */ {BUILDING_GOVERNMENT_LOGISTICS,          ST_IRON,   DEV_NONE, -1,  180,  100,    0,    0,  60, -1, -1},
    // -------------------- 神殿 --------------------
    /* 88 造祭司        */ {BUILDING_TEMPLE_CREATE_PRIEST,         ST_BRONZE, DEV_NONE, -1,    0,  125,    0,    0,  50, SORT_ARMY, AT_PRIEST},
    /* 89 多神教        */ {BUILDING_TEMPLE_POLYTHEISM,            ST_TOOL,   DEV_NONE, -1,    0,  120,    0,    0,  50, -1, -1},
    /* 90 神秘主义      */ {BUILDING_TEMPLE_MYSTICISM,             ST_BRONZE, DEV_T(TECH_TEMPLE_POLYTHEISM),          -1,    0,  120,    0,    0,  50, -1, -1},
    /* 91 占星术        */ {BUILDING_TEMPLE_ASTROLOGY,             ST_BRONZE, DEV_NONE, -1,    0,  175,    0,    0,  50, -1, -1},
    /* 92 医学          */ {BUILDING_TEMPLE_MEDICINE,              ST_BRONZE, DEV_NONE, -1,    0,  150,    0,    0,  50, -1, -1},
    /* 93 一神教        */ {BUILDING_TEMPLE_MONOTHEISM,            ST_IRON,   DEV_T(TECH_TEMPLE_MYSTICISM),           -1,    0,  350,    0,    0,  75, -1, -1},
    /* 94 来生          */ {BUILDING_TEMPLE_AFTERLIFE,             ST_IRON,   DEV_T(TECH_TEMPLE_ASTROLOGY),           -1,    0,  275,    0,    0,  75, -1, -1},
    /* 95 狂热          */ {BUILDING_TEMPLE_FANATICISM,            ST_IRON,   DEV_T(TECH_TEMPLE_MONOTHEISM),           -1,    0,  150,    0,    0,  60, -1, -1},
    /* 96 狂热献祭      */ {BUILDING_TEMPLE_ZEALOTRY,              ST_IRON,   DEV_T(TECH_TEMPLE_FANATICISM),           -1,    0,  120,    0,    0,  60, -1, -1},
    /* 97 献祭          */ {BUILDING_TEMPLE_SACRIFICE,             ST_IRON,   DEV_T(TECH_TEMPLE_FANATICISM),           -1,    0,  400,    0,    0, 100, -1, -1},
};

constexpr I32 ACTION_COUNT = static_cast<I32>(sizeof(kActions) / sizeof(kActions[0]));

/*---------------------------- 建筑规则 ----------------------------*/
struct BuildingRule
{
    I32 building;
    I32 stage;
    TechnologyBits requireTech;
    I32 requireBuilding;   // 前置建筑，-1 表示无
    I32 food;
    I32 gold;
    I32 wood;
    I32 stone;
    I32 seconds;
};

constexpr BuildingRule kBuildings[BUILDING_TYPE_MAXNUM] = {
    /*  0 房屋          */ {BUILDING_HOME,              ST_STONE,  DEV_NONE, BUILDING_CENTER,    0, 0,   30,    0,  20},
    /*  1 谷仓          */ {BUILDING_GRANARY,           ST_STONE,  DEV_NONE, BUILDING_CENTER,    0, 0,  120,    0,  30},
    /*  2 市镇中心      */ {BUILDING_CENTER,            ST_STONE,  DEV_NONE, -1,                 0, 0,  200,    0,  60},
    /*  3 仓库          */ {BUILDING_STOCK,             ST_STONE,  DEV_NONE, BUILDING_CENTER,    0, 0,  120,    0,  30},
    /*  4 农场          */ {BUILDING_FARM,              ST_TOOL,   DEV_NONE, BUILDING_MARKET,    0, 0,   75,    0,  30},
    /*  5 市场          */ {BUILDING_MARKET,            ST_TOOL,   DEV_NONE, BUILDING_GRANARY,   0, 0,  150,    0,  40},
    /*  6 瞭望箭塔      */ {BUILDING_WATCH_TOWER,       ST_TOOL,   DEV_T(TECH_GRANARY_ARROWTOWER),          -1, 0, 0, 0, 150, 80},
    /*  7 警戒箭塔      */ {BUILDING_SENTRY_TOWER,      ST_BRONZE, DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_1),-1, 0, 0, 0, 150, 80},
    /*  8 防御箭塔      */ {BUILDING_GUARD_TOWER,       ST_IRON,   DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_2),-1, 0, 0, 0, 150, 80},
    /*  9 防御炮塔      */ {BUILDING_BALLISTA_TOWER,    ST_IRON,   DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_3),-1, 0, 0, 0, 150, 80},
    /* 10 军营          */ {BUILDING_ARMYCAMP,          ST_STONE,  DEV_NONE, BUILDING_CENTER,    0, 0,  125,    0,  30},
    /* 11 马厩          */ {BUILDING_STABLE,            ST_TOOL,   DEV_NONE, BUILDING_ARMYCAMP,  0, 0,  150,    0,  40},
    /* 12 靶场          */ {BUILDING_RANGE,             ST_TOOL,   DEV_NONE, BUILDING_ARMYCAMP,  0, 0,  150,    0,  40},
    /* 13 船坞          */ {BUILDING_DOCK,              ST_STONE,  DEV_NONE, BUILDING_CENTER,    0, 0,  100,    0,  50},
    /* 14 攻城武器厂    */ {BUILDING_SIEGE,             ST_BRONZE, DEV_NONE, BUILDING_RANGE,     0, 0,  200,    0,  60},
    /* 15 学院          */ {BUILDING_COLLAGE,           ST_BRONZE, DEV_NONE, BUILDING_STABLE,    0, 0,  200,    0,  60},
    /* 16 政府中心      */ {BUILDING_GOVERNMENT_CENTER, ST_BRONZE, DEV_NONE, BUILDING_MARKET,    0, 0,  175,    0,  60},
    /* 17 世界奇观      */ {BUILDING_WONDER,            ST_IRON,   DEV_NONE, -1,                 0, 0, 1000, 1000, 7200},
    /* 18 神庙          */ {BUILDING_TEMPLE,            ST_BRONZE, DEV_NONE, BUILDING_MARKET,    0, 0,  200,    0,  60},
    /* 19 初级城墙      */ {BUILDING_SMALL_WALL,        ST_TOOL,   DEV_T(TECH_GRANARY_WALL),               -1, 0, 0, 0,   5,   8},
    /* 20 中级城墙      */ {BUILDING_MEDIUM_WALL,       ST_BRONZE, DEV_T(TECH_GRANARY_WALL_UPDATE_1),      -1, 0, 0, 0,   5,   8},
    /* 21 高级城墙      */ {BUILDING_FORTIFIED_WALL,    ST_IRON,   DEV_T(TECH_GRANARY_WALL_UPDATE_2),      -1, 0, 0, 0,   5,   8},
};

/*---------------------------- 文明掩码 ----------------------------
 * 每个文明「不能研究 / 不能建造」的东西。
 * 数据来源：文明加成与开局表里的「缺失科技」「缺失单位」两列。
 *
 * 征兵(Conscription)、城市化(Urbanization)、城防(City Watch)、
 * 神权政治(Theocracy) 这 4 项经讨论确认不纳入实现范围：它们在
 * BuildingAction 里没有对应动作，本模块也就不需要表达它们。下面各文明的
 * 注释里即使列了这几项，也不会出现在掩码位里 —— 这不是遗漏。
 *------------------------------------------------------------------*/
struct CivilizationRule
{
    I32 civilization;
    TechnologyBits blockedTech;
    U64 blockedBuilding;   // 按 BUILDING_TYPE 索引，1 位一个建筑
};

constexpr U64 bldBit(I32 building)
{
    return (building >= 0 && building < BUILDING_TYPE_MAXNUM) ? (U64(1) << building) : 0;
}

constexpr CivilizationRule kCivilizations[] = {
    // 亚述：青铜盾、贵族、建筑学、城防、贵族政治
    {CIVILIZATION_ASSYRIAN,
     DEV_T(TECH_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY) | DEV_T(TECH_GOVERNMENT_NOBILITY)
     | DEV_T(TECH_GOVERNMENT_ARCHITECTURE) | DEV_T(TECH_GOVERNMENT_ARISTOCRACY), 0},
    // 巴比伦：铁盾
    {CIVILIZATION_BABYLONIAN,
     DEV_T(TECH_STOCK_UPGRADE_IRON_SHIELD), 0},
    // 迦太基：冶金、锁子甲（步/弓/骑）、加固城墙、攻城术、占星术、一神教、狂热
    {CIVILIZATION_CARTHAGINIAN,
     DEV_T(TECH_STOCK_UPGRADE_METALLURGY)
     | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_3) | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_ARCHER_3)
     | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_RIDER_3) | DEV_T(TECH_GRANARY_WALL_UPDATE_2)
     | DEV_T(TECH_MARKET_SIEGECRAFT) | DEV_T(TECH_TEMPLE_ASTROLOGY)
     | DEV_T(TECH_TEMPLE_MONOTHEISM) | DEV_T(TECH_TEMPLE_FANATICISM), 0},
    // 朝鲜：锁子甲（步/弓/骑）、铁盾、贵族政治、炼金术、工程学
    {CIVILIZATION_ANCIENT_CHOSON,
     DEV_T(TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_3) | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_ARCHER_3)
     | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_RIDER_3) | DEV_T(TECH_STOCK_UPGRADE_IRON_SHIELD)
     | DEV_T(TECH_GOVERNMENT_ARISTOCRACY) | DEV_T(TECH_GOVERNMENT_ALCHEMY)
     | DEV_T(TECH_GOVERNMENT_ENGINEERING), 0},
    // 埃及：青铜盾、攻城术、城防
    {CIVILIZATION_EGYPTIAN,
     DEV_T(TECH_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY) | DEV_T(TECH_MARKET_SIEGECRAFT), 0},
    // 希腊：冶金、城防、征兵、一神教、狂热
    {CIVILIZATION_GREEK,
     DEV_T(TECH_STOCK_UPGRADE_METALLURGY) | DEV_T(TECH_TEMPLE_MONOTHEISM)
     | DEV_T(TECH_TEMPLE_FANATICISM), 0},
    // 赫梯：灌溉、建筑学、神秘主义、多神教、来世、一神教、狂热、狂热献祭、医学
    {CIVILIZATION_HITTITE,
     DEV_T(TECH_MARKET_IRRIGATION) | DEV_T(TECH_GOVERNMENT_ARCHITECTURE)
     | DEV_T(TECH_TEMPLE_MYSTICISM) | DEV_T(TECH_TEMPLE_POLYTHEISM)
     | DEV_T(TECH_TEMPLE_AFTERLIFE) | DEV_T(TECH_TEMPLE_MONOTHEISM)
     | DEV_T(TECH_TEMPLE_FANATICISM) | DEV_T(TECH_TEMPLE_ZEALOTRY)
     | DEV_T(TECH_TEMPLE_MEDICINE), 0},
    // 马其顿：加固城墙、工艺、攻城术、贵族、工程学；无神庙
    {CIVILIZATION_MACEDONIAN,
     DEV_T(TECH_GRANARY_WALL_UPDATE_2) | DEV_T(TECH_MARKET_CRAFT_UPGRADE)
     | DEV_T(TECH_MARKET_SIEGECRAFT) | DEV_T(TECH_GOVERNMENT_NOBILITY)
     | DEV_T(TECH_GOVERNMENT_ENGINEERING),
     bldBit(BUILDING_TEMPLE)},
    // 米诺：加固城墙、守卫塔、占星术、神秘主义、来世、一神教、狂热、狂热献祭
    {CIVILIZATION_MINOAN,
     DEV_T(TECH_GRANARY_WALL_UPDATE_2) | DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_1)
     | DEV_T(TECH_TEMPLE_ASTROLOGY) | DEV_T(TECH_TEMPLE_MYSTICISM)
     | DEV_T(TECH_TEMPLE_AFTERLIFE) | DEV_T(TECH_TEMPLE_MONOTHEISM)
     | DEV_T(TECH_TEMPLE_FANATICISM) | DEV_T(TECH_TEMPLE_ZEALOTRY), 0},
    // 帕尔米拉：冶金、塔盾、工艺、灌溉、后勤、贵族政治、工程学、神秘主义、多神教、一神教、医学
    {CIVILIZATION_PALMYRENEAN,
     DEV_T(TECH_STOCK_UPGRADE_METALLURGY) | DEV_T(TECH_STOCK_UPGRADE_TOWER_SHIELD)
     | DEV_T(TECH_MARKET_CRAFT_UPGRADE) | DEV_T(TECH_MARKET_IRRIGATION)
     | DEV_T(TECH_GOVERNMENT_LOGISTICS) | DEV_T(TECH_GOVERNMENT_ARISTOCRACY)
     | DEV_T(TECH_GOVERNMENT_ENGINEERING) | DEV_T(TECH_TEMPLE_MYSTICISM)
     | DEV_T(TECH_TEMPLE_POLYTHEISM) | DEV_T(TECH_TEMPLE_MONOTHEISM)
     | DEV_T(TECH_TEMPLE_MEDICINE), 0},
    // 波斯：无学院、弩炮塔、工艺、攻城术、灌溉、贵族政治
    {CIVILIZATION_PERSIAN,
     DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_3) | DEV_T(TECH_MARKET_CRAFT_UPGRADE)
     | DEV_T(TECH_MARKET_SIEGECRAFT) | DEV_T(TECH_MARKET_IRRIGATION)
     | DEV_T(TECH_GOVERNMENT_ARISTOCRACY),
     bldBit(BUILDING_COLLAGE)},
    // 腓尼基：冶金、锁子甲（步/弓/骑）、攻城术、建筑学
    {CIVILIZATION_PHOENICIAN,
     DEV_T(TECH_STOCK_UPGRADE_METALLURGY)
     | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_3) | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_ARCHER_3)
     | DEV_T(TECH_STOCK_UPGRADE_DEFENSE_RIDER_3) | DEV_T(TECH_MARKET_SIEGECRAFT)
     | DEV_T(TECH_GOVERNMENT_ARCHITECTURE), 0},
    // 罗马：守卫塔、灌溉、炼金术、占星术、来世
    {CIVILIZATION_ROMAN,
     DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_1) | DEV_T(TECH_MARKET_IRRIGATION)
     | DEV_T(TECH_GOVERNMENT_ALCHEMY) | DEV_T(TECH_TEMPLE_ASTROLOGY)
     | DEV_T(TECH_TEMPLE_AFTERLIFE), 0},
    // 商：弩炮塔、攻城术、贵族政治、炼金术、工程学
    {CIVILIZATION_SHANG,
     DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_3) | DEV_T(TECH_MARKET_SIEGECRAFT)
     | DEV_T(TECH_GOVERNMENT_ARISTOCRACY) | DEV_T(TECH_GOVERNMENT_ALCHEMY)
     | DEV_T(TECH_GOVERNMENT_ENGINEERING), 0},
    // 苏美尔：冶金、铁盾、工艺、占星术、来世、一神教、狂热、狂热献祭
    {CIVILIZATION_SUMERIAN,
     DEV_T(TECH_STOCK_UPGRADE_METALLURGY) | DEV_T(TECH_STOCK_UPGRADE_IRON_SHIELD)
     | DEV_T(TECH_MARKET_CRAFT_UPGRADE) | DEV_T(TECH_TEMPLE_ASTROLOGY)
     | DEV_T(TECH_TEMPLE_AFTERLIFE) | DEV_T(TECH_TEMPLE_MONOTHEISM)
     | DEV_T(TECH_TEMPLE_FANATICISM) | DEV_T(TECH_TEMPLE_ZEALOTRY), 0},
    // 大和：加固城墙、守卫塔、塔盾、城市化、占星术、神秘主义、一神教、狂热、狂热献祭、医学
    {CIVILIZATION_YAMATO,
     DEV_T(TECH_GRANARY_WALL_UPDATE_2) | DEV_T(TECH_GRANARY_ARROWTOWER_UPGRADE_1)
     | DEV_T(TECH_STOCK_UPGRADE_TOWER_SHIELD) | DEV_T(TECH_TEMPLE_ASTROLOGY)
     | DEV_T(TECH_TEMPLE_MYSTICISM) | DEV_T(TECH_TEMPLE_MONOTHEISM)
     | DEV_T(TECH_TEMPLE_FANATICISM) | DEV_T(TECH_TEMPLE_ZEALOTRY)
     | DEV_T(TECH_TEMPLE_MEDICINE), 0},
};

// 按 action 编号取规则（表按编号顺序排列，直接下标）
const ActionRule* findAction(I32 action)
{
    if (action <= 0 || action > ACTION_COUNT) {
        return nullptr;
    }
    const ActionRule& rule = kActions[action - 1];
    if (rule.action != action) {
        return nullptr;   // 表写错了，宁可判为不可用也不要静默返回别的规则
    }
    return &rule;
}

I32 secondsToTicks(I32 seconds)
{
    if (seconds <= 0) {
        return 0;
    }
    return seconds * GAME_TPS;
}

// 该 sort 是否算「近战单位」（受近战攻击/护甲科技影响）
bool isMeleeSort(I32 sort)
{
    return sort == SORT_ARMY || sort == SORT_FARMER;
}

} // namespace

/*==========================================================================
 * 构造与科技状态
 *========================================================================*/

Development::Development(WeakPtr<Player> playerValue, I32 civilizationValue)
    : player(playerValue),
      stage(STAGE_MIN),
      civilization(civilizationValue),
      technology(),
      blocked(),
      blockedBuilding(0)
{
    for (const CivilizationRule& rule : kCivilizations) {
        if (rule.civilization == civilization) {
            blocked = rule.blockedTech;
            blockedBuilding = rule.blockedBuilding;
            break;
        }
    }
}

bool Development::TechnologyFinish(I32 technologyId) const
{
    return technology.has(technologyId);
}

bool Development::TechnologyBlocked(I32 technologyId) const
{
    return blocked.has(technologyId);
}

bool Development::markTechnologyFinish(I32 action)
{
    const ActionRule* rule = findAction(action);
    if (rule == nullptr) {
        return false;
    }
    // 造人动作不产生科技位
    if (rule->createSort >= 0) {
        return false;
    }
    if (TechnologyBlocked(action)) {
        return false;
    }
    if (technology.has(action)) {
        return false;
    }
    technology.add(action);

    // 时代升级单独处理：动作完成即推进时代
    if (action == BUILDING_CENTER_UPGRADE_TOOLAGE
        || action == BUILDING_CENTER_UPGRADE_BRONZEAGE
        || action == BUILDING_CENTER_UPGRADE_IRONAGE) {
        stageChange();
    }
    return true;
}

I32 Development::getStage() const
{
    return stage;
}

I32 Development::getCivilization() const
{
    return civilization;
}

/*==========================================================================
 * 加成查询
 *
 * 统一口径：比例类返回「乘数」（1 表示无加成），数值类返回「加数」。
 * 具体数值来自科技总表与满科技叠加表；两者冲突时以科技总表逐条为准，
 * 冲突处已加注释。
 *========================================================================*/

Double Development::get_rate_Move(I32 sort, I32 type) const
{
    // 车轮：村民移速 +50%
    if (sort == SORT_FARMER && type == FARMERTYPE_FARMER
        && technology.has(TECH_MARKET_WHEEL_UPGRADE)) {
        return Double(3) / Double(2);
    }
    // 贵族政治：学院单位移速 +0.225 区块/秒。
    // 学院单位基础移速 0.9 区块/秒，0.225 / 0.9 = 25%，所以是 ×1.25。
    if (sort == SORT_ARMY
        && (type == AT_HOPLITE || type == AT_PHALANX || type == AT_CENTURION)
        && technology.has(TECH_GOVERNMENT_ARISTOCRACY)) {
        return Double(5) / Double(4);
    }
    // 多神教：祭司移速 +40%
    if (sort == SORT_ARMY && type == AT_PRIEST
        && technology.has(TECH_TEMPLE_POLYTHEISM)) {
        return Double(7) / Double(5);
    }
    return Double(1);
}

Double Development::get_rate_Blood(I32 sort, I32 type) const
{
    // 贵族：骑兵（除象）生命 +15%
    if (sort == SORT_ARMY && technology.has(TECH_GOVERNMENT_NOBILITY)) {
        switch (type) {
        case AT_SCOUT:
        case AT_CAVALRY:
        case AT_HEAVY_CAVALRY:
        case AT_CATAPHRACT:
        case AT_CAMEL_RIDER:
        case AT_CHARIOT:
        case AT_SCYTHECHARIOT:
            return Double(23) / Double(20);   // 1.15
        default:
            break;
        }
    }
    // 建筑学：建筑生命 +20%
    if (sort == SORT_BUILDING || sort == SORT_BUILDING_RESOURCE) {
        if (technology.has(TECH_GOVERNMENT_ARCHITECTURE)) {
            return Double(6) / Double(5);
        }
    }
    return Double(1);
}

I32 Development::get_addition_Blood(I32 sort, I32 type) const
{
    I32 result = 0;
    // 神秘主义：祭司生命 +20
    if (sort == SORT_ARMY && type == AT_PRIEST && technology.has(TECH_TEMPLE_MYSTICISM)) {
        result += 20;
    }
    // 狂热献祭（Zealotry）：村民攻击 +7、生命 +40、移速 +0.11（移速项未实现）
    if (sort == SORT_FARMER && type == FARMERTYPE_FARMER && technology.has(TECH_TEMPLE_ZEALOTRY)) {
        result += 40;
    }
    return result;
}

Double Development::get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    // 现有科技对攻击都是数值加成，没有比例加成
    (void)sort; (void)type; (void)armyClass; (void)attackType;
    return Double(1);
}

I32 Development::get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    I32 result = 0;
    if (!isMeleeSort(sort)) {
        return 0;
    }

    if (attackType == ATTACKTYPE_SHOOT) {
        // 炼金术：远程攻击 +1
        if (technology.has(TECH_GOVERNMENT_ALCHEMY)) {
            result += 1;
        }
        // 采石 / 攻城术：投石兵攻击 +1 各一次
        if (sort == SORT_ARMY && type == AT_SLINGER) {
            if (technology.has(TECH_MARKET_STONE_UPGRADE)) {
                result += 1;
            }
            if (technology.has(TECH_MARKET_SIEGECRAFT)) {
                result += 1;
            }
        }
        // 炼金术：火焰船攻击 +6（火帆船按近战处理时在下面加）
        if (sort == SORT_ARMY && type == AT_FIRESAILBOAT
            && technology.has(TECH_GOVERNMENT_ALCHEMY)) {
            result += 6;
        }
        return result;
    }

    // 近战：工具锻造 +2、金属加工 +2、冶金 +3
    if (sort == SORT_ARMY || sort == SORT_FARMER) {
        if (technology.has(TECH_STOCK_UPGRADE_USETOOL)) {
            result += 2;
        }
        if (technology.has(TECH_STOCK_UPGRADE_METALWORKING)) {
            result += 2;
        }
        if (technology.has(TECH_STOCK_UPGRADE_METALLURGY)) {
            result += 3;
        }
    }
    // 狂热献祭：村民攻击 +7
    if (sort == SORT_FARMER && type == FARMERTYPE_FARMER
        && technology.has(TECH_TEMPLE_ZEALOTRY)) {
        result += 7;
    }
    (void)armyClass;
    return result;
}

I32 Development::get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const
{
    I32 result = 0;

    // 祭司：来生 +3
    if (sort == SORT_ARMY && type == AT_PRIEST) {
        if (technology.has(TECH_TEMPLE_AFTERLIFE)) {
            result += 3;
        }
        return result;
    }

    // 攻城器械：工程学 +2
    if (sort == SORT_ARMY && armyClass == ARMY_SIEGE) {
        if (technology.has(TECH_GOVERNMENT_ENGINEERING)) {
            result += 2;
        }
    }

    if (attackType == ATTACKTYPE_SHOOT) {
        // 木工 / 工匠技艺 / 工艺：远程射程 +1 各一次
        if (technology.has(TECH_MARKET_WOOD_UPGRADE)) {
            result += 1;
        }
        if (technology.has(TECH_MARKET_ARTISANSHIP)) {
            result += 1;
        }
        if (technology.has(TECH_MARKET_CRAFT_UPGRADE)) {
            result += 1;
        }
        // 采石 / 攻城术：投石兵射程 +1 各一次
        if (sort == SORT_ARMY && type == AT_SLINGER) {
            if (technology.has(TECH_MARKET_STONE_UPGRADE)) {
                result += 1;
            }
            if (technology.has(TECH_MARKET_SIEGECRAFT)) {
                result += 1;
            }
        }
    }
    return result;
}

Double Development::get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    // 现有防御科技都是数值加成
    (void)sort; (void)type; (void)armyClass; (void)attackType_got;
    return Double(1);
}

I32 Development::get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const
{
    I32 result = 0;
    if (sort != SORT_ARMY && sort != SORT_FARMER) {
        return 0;
    }

    if (attackType_got == ATTACKTYPE_SHOOT) {
        // 穿刺护甲：青铜盾 / 铁盾 / 塔盾 各 +1，只作用于步兵与投石兵
        if (armyClass == ARMY_INFANTRY) {
            if (technology.has(TECH_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY)) {
                result += 1;
            }
            if (technology.has(TECH_STOCK_UPGRADE_IRON_SHIELD)) {
                result += 1;
            }
            if (technology.has(TECH_STOCK_UPGRADE_TOWER_SHIELD)) {
                result += 1;
            }
        }
        return result;
    }

    // 近战护甲：按兵种大类各有一条三级科技链，每级 +2
    I32 techFirst = -1;
    switch (armyClass) {
    case ARMY_INFANTRY: techFirst = TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_1; break;
    case ARMY_ARCHER:   techFirst = TECH_STOCK_UPGRADE_DEFENSE_ARCHER_1;   break;
    case ARMY_RIDER:    techFirst = TECH_STOCK_UPGRADE_DEFENSE_RIDER_1;    break;
    default: break;
    }
    if (techFirst < 0) {
        return 0;
    }
    if (technology.has(techFirst)) {
        result += 2;
    }
    if (technology.has(techFirst + 1)) {
        result += 2;
    }
    if (technology.has(techFirst + 2)) {
        result += 2;
    }
    (void)type;
    return result;
}

I32 Development::get_addition_ResourceCarry(I32 resourceSort) const
{
    switch (resourceSort) {
    case RESOURCESORT_WOOD:
        // 木工 / 工匠技艺 / 工艺 各 +2
        return (technology.has(TECH_MARKET_WOOD_UPGRADE) ? 2 : 0)
             + (technology.has(TECH_MARKET_ARTISANSHIP) ? 2 : 0)
             + (technology.has(TECH_MARKET_CRAFT_UPGRADE) ? 2 : 0);
    case RESOURCESORT_STONE:
        // 采石 +3
        return technology.has(TECH_MARKET_STONE_UPGRADE) ? 3 : 0;
    case RESOURCESORT_GOLD:
        // 采金 +3
        return technology.has(TECH_MARKET_GOLD_UPGRADE) ? 3 : 0;
    default:
        return 0;
    }
}

Double Development::get_rate_ResourceGather(I32 resourceSort) const
{
    switch (resourceSort) {
    case RESOURCESORT_WOOD: {
        // 木工 / 工匠技艺 / 工艺 各 +20%
        I32 percent = 0;
        if (technology.has(TECH_MARKET_WOOD_UPGRADE))  percent += 20;
        if (technology.has(TECH_MARKET_ARTISANSHIP))   percent += 20;
        if (technology.has(TECH_MARKET_CRAFT_UPGRADE)) percent += 20;
        return (Double(100) + Double(percent)) / Double(100);
    }
    case RESOURCESORT_STONE: {
        // 采石 / 攻城术 各 +30%
        I32 percent = 0;
        if (technology.has(TECH_MARKET_STONE_UPGRADE)) percent += 30;
        if (technology.has(TECH_MARKET_SIEGECRAFT))    percent += 30;
        return (Double(100) + Double(percent)) / Double(100);
    }
    case RESOURCESORT_GOLD: {
        // 采金 +30%（铸币是金矿产量 +25%，不影响采集速度）
        I32 percent = technology.has(TECH_MARKET_GOLD_UPGRADE) ? 30 : 0;
        return (Double(100) + Double(percent)) / Double(100);
    }
    default:
        return Double(1);
    }
}

I32 Development::get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const
{
    // 农田：驯养 +75、犁 +100、灌溉 +125
    if ((sort == SORT_BUILDING_RESOURCE || sort == SORT_BUILDING) && type == BUILDING_FARM) {
        I32 extra = 0;
        if (technology.has(TECH_MARKET_FARM_UPGRADE)) extra += 75;
        if (technology.has(TECH_MARKET_PLOW_UPGRADE)) extra += 100;
        if (technology.has(TECH_MARKET_IRRIGATION))   extra += 125;
        return extra;
    }
    return 0;
}

/*==========================================================================
 * 时代
 *========================================================================*/

void Development::stageChange()
{
    if (stage >= STAGE_MAX) {
        return;
    }
    stage += 1;
}

bool Development::stageCanChange() const
{
    // 下一个时代对应的升级动作是否还没完成、且前置科技齐备
    I32 nextAction = -1;
    switch (stage) {
    case ST_STONE:  nextAction = BUILDING_CENTER_UPGRADE_TOOLAGE;  break;
    case ST_TOOL:   nextAction = BUILDING_CENTER_UPGRADE_BRONZEAGE; break;
    case ST_BRONZE: nextAction = BUILDING_CENTER_UPGRADE_IRONAGE;  break;
    default: return false;
    }
    const ActionRule* rule = findAction(nextAction);
    if (rule == nullptr) {
        return false;
    }
    if (TechnologyBlocked(nextAction)) {
        return false;
    }
    return technology.contains(rule->requireTech);
}

/*==========================================================================
 * 建筑
 *========================================================================*/

bool Development::buildingActionAble(I32 buildingType, I32 actNum,
                                     I32 wood, I32 food, I32 stone, I32 gold) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    if (wood < 0 || food < 0 || stone < 0 || gold < 0) {
        return false;
    }

    const ActionRule* rule = findAction(actNum);
    if (rule == nullptr) {
        return false;
    }

    // 这个动作是否属于该建筑（表里没写建筑字段，用动作编号分段判断由内核保证；
    // 这里只校验动作本身的前置条件）

    // 1. 文明封锁
    if (TechnologyBlocked(actNum)) {
        return false;
    }

    // 2. 时代
    if (stage < rule->stage) {
        return false;
    }

    // 3. 前置科技：已完成的科技必须包含规则要求的全部位
    if (!technology.contains(rule->requireTech)) {
        return false;
    }

    // 4. 前置建筑
    if (rule->requireBuilding >= 0 && !hasConstructedBuilding(rule->requireBuilding)) {
        return false;
    }

    // 5. 资源
    if (wood < rule->wood || food < rule->food
        || stone < rule->stone || gold < rule->gold) {
        return false;
    }

    return true;
}

I32 Development::buildingActionTimeCost(I32 buildingType, I32 actNum) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return 0;
    }
    const ActionRule* rule = findAction(actNum);
    if (rule == nullptr) {
        return 0;
    }
    return secondsToTicks(rule->seconds);
}

bool Development::buildingCanBuild(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return false;
    }
    // 文明封锁的建筑
    if ((blockedBuilding & bldBit(buildingType)) != 0) {
        return false;
    }

    const BuildingRule& rule = kBuildings[buildingType];
    if (rule.building != buildingType) {
        return false;   // 表写错
    }

    if (stage < rule.stage) {
        return false;
    }
    if (!technology.contains(rule.requireTech)) {
        return false;
    }
    if (rule.requireBuilding >= 0 && !hasConstructedBuilding(rule.requireBuilding)) {
        return false;
    }
    return true;
}

I32 Development::buildingBuildTimeCost(I32 buildingType) const
{
    if (buildingType < 0 || buildingType >= BUILDING_TYPE_MAXNUM) {
        return 0;
    }
    const BuildingRule& rule = kBuildings[buildingType];
    if (rule.building != buildingType) {
        return 0;
    }

    I32 seconds = rule.seconds;
    // 建筑学：建造时间 -33%
    if (technology.has(TECH_GOVERNMENT_ARCHITECTURE)) {
        seconds = seconds * 67 / 100;
    }
    return secondsToTicks(seconds);
}

void Development::tick()
{
    // 科技状态由 markTechnologyFinish 在建筑动作完成时更新，这里暂不需要逐帧处理
}

/*==========================================================================
 * 内部工具
 *========================================================================*/

bool Development::hasConstructedBuilding(I32 type) const
{
    SafePtr<Player> locked = player.lock();
    if (locked == nullptr) {
        return false;
    }
    for (const SafePtr<Building>& building : locked->getBuilding()) {
        if (!building.IsNull() && building->getType() == type && building->isConstructed()) {
            return true;
        }
    }
    for (const SafePtr<BuildingResource>& resource : locked->getBuildingResource()) {
        if (!resource.IsNull() && resource->getType() == type && resource->isConstructed()) {
            return true;
        }
    }
    return false;
}
