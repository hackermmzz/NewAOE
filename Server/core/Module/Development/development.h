#pragma once

#include "Server/core/type.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Civilization.h"

class Player;

/*==========================================================================
 * 科技枚举
 *
 * 每个科技的编号直接等于它对应的 BuildingAction 编号，所以「按 Action 建表」
 * 不需要任何映射：位图下标就是 Action 值，前置条件里写的也是同一个编号。
 * 造人类的动作（BUILDING_*_CREATE_*）不产生科技，不在这个枚举里。
 *========================================================================*/
enum Technology : I32
{
    TECH_NONE = 0,

    // ---------- 市镇中心 / 粮仓：时代与解锁 ----------
    TECH_CENTER_UPGRADE_TOOLAGE = BUILDING_CENTER_UPGRADE_TOOLAGE,
    TECH_CENTER_UPGRADE_BRONZEAGE = BUILDING_CENTER_UPGRADE_BRONZEAGE,
    TECH_CENTER_UPGRADE_IRONAGE = BUILDING_CENTER_UPGRADE_IRONAGE,

    TECH_GRANARY_WALL = BUILDING_GRANARY_WALL,
    TECH_GRANARY_WALL_UPDATE_1 = BUILDING_GRANARY_WALL_UPDATE_1,
    TECH_GRANARY_WALL_UPDATE_2 = BUILDING_GRANARY_WALL_UPDATE_2,

    TECH_GRANARY_ARROWTOWER = BUILDING_GRANARY_ARROWTOWER,
    TECH_GRANARY_ARROWTOWER_UPGRADE_1 = BUILDING_GRANARY_ARROWTOWER_UPGRADE_1,
    TECH_GRANARY_ARROWTOWER_UPGRADE_2 = BUILDING_GRANARY_ARROWTOWER_UPGRADE_2,
    TECH_GRANARY_ARROWTOWER_UPGRADE_3 = BUILDING_GRANARY_ARROWTOWER_UPGRADE_3,

    // ---------- 市场：经济 ----------
    TECH_MARKET_WOOD_UPGRADE = BUILDING_MARKET_WOOD_UPGRADE,
    TECH_MARKET_ARTISANSHIP = BUILDING_MARKET_ARTISANSHIP,
    TECH_MARKET_CRAFT_UPGRADE = BUILDING_MARKET_CRAFT_UPGRADE,

    TECH_MARKET_STONE_UPGRADE = BUILDING_MARKET_STONE_UPGRADE,
    TECH_MARKET_SIEGECRAFT = BUILDING_MARKET_SIEGECRAFT,

    TECH_MARKET_GOLD_UPGRADE = BUILDING_MARKET_GOLD_UPGRADE,
    TECH_MARKET_COINAGE = BUILDING_MARKET_COINAGE,

    TECH_MARKET_FARM_UPGRADE = BUILDING_MARKET_FARM_UPGRADE,
    TECH_MARKET_PLOW_UPGRADE = BUILDING_MARKET_PLOW_UPGRADE,
    TECH_MARKET_IRRIGATION = BUILDING_MARKET_IRRIGATION,

    TECH_MARKET_WHEEL_UPGRADE = BUILDING_MARKET_WHEEL_UPGRADE,

    // ---------- 仓库：军事 ----------
    TECH_STOCK_UPGRADE_USETOOL = BUILDING_STOCK_UPGRADE_USETOOL,
    TECH_STOCK_UPGRADE_METALWORKING = BUILDING_STOCK_UPGRADE_METALWORKING,
    TECH_STOCK_UPGRADE_METALLURGY = BUILDING_STOCK_UPGRADE_METALLURGY,

    TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_1 = BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_1,
    TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_2 = BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_2,
    TECH_STOCK_UPGRADE_DEFENSE_INFANTRY_3 = BUILDING_STOCK_UPGRADE_DEFENSE_INFANTRY_3,

    TECH_STOCK_UPGRADE_DEFENSE_ARCHER_1 = BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_1,
    TECH_STOCK_UPGRADE_DEFENSE_ARCHER_2 = BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_2,
    TECH_STOCK_UPGRADE_DEFENSE_ARCHER_3 = BUILDING_STOCK_UPGRADE_DEFENSE_ARCHER_3,

    TECH_STOCK_UPGRADE_DEFENSE_RIDER_1 = BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_1,
    TECH_STOCK_UPGRADE_DEFENSE_RIDER_2 = BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_2,
    TECH_STOCK_UPGRADE_DEFENSE_RIDER_3 = BUILDING_STOCK_UPGRADE_DEFENSE_RIDER_3,

    TECH_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY = BUILDING_STOCK_UPGRADE_MISSILE_DEFENSE_INFANTRY,
    TECH_STOCK_UPGRADE_IRON_SHIELD = BUILDING_STOCK_UPGRADE_IRON_SHIELD,
    TECH_STOCK_UPGRADE_TOWER_SHIELD = BUILDING_STOCK_UPGRADE_TOWER_SHIELD,

    // ---------- 兵营：步兵升级 ----------
    TECH_ARMYCAMP_UPGRADE_CLUBMAN = BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,
    TECH_ARMYCAMP_UPGRADE_BROADSWORD = BUILDING_ARMYCAMP_UPGRADE_BROADSWORD,
    TECH_ARMYCAMP_UPGRADE_LONGSWORD = BUILDING_ARMYCAMP_UPGRADE_LONGSWORD,
    TECH_ARMYCAMP_UPGRADE_LEGION = BUILDING_ARMYCAMP_UPGRADE_LEGION,

    // ---------- 靶场：弓兵升级 ----------
    TECH_RANGE_UPGRADE_IMPROVEDBOW = BUILDING_RANGE_UPGRADE_IMPROVEDBOW,
    TECH_RANGE_UPGRADE_COMPOSITE_BOW = BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,
    TECH_RANGE_UPGRADE_HORSE_ARCHER = BUILDING_RANGE_UPGRADE_HORSE_ARCHER,

    // ---------- 马厩：骑兵升级 ----------
    TECH_STABLE_UPGRADE_CAVALRY = BUILDING_STABLE_UPGRADE_CAVALRY,
    TECH_STABLE_UPGRADE_CATAPHRACT = BUILDING_STABLE_UPGRADE_CATAPHRACT,
    TECH_STABLE_UPGRADE_CHARIOT = BUILDING_STABLE_UPGRADE_CHARIOT,
    TECH_STABLE_UPGRADE_ELEPHANT = BUILDING_STABLE_UPGRADE_ELEPHANT,

    // ---------- 船坞：船只升级 ----------
    TECH_DOCK_UPGRADE_SAILING = BUILDING_DOCK_UPGRADE_SAILING,
    TECH_DOCK_UPGRADE_TRADEBOAT = BUILDING_DOCK_UPGRADE_TRADEBOAT,
    TECH_DOCK_UPGRADE_TRANSPORT = BUILDING_DOCK_UPGRADE_TRANSPORT,
    TECH_DOCK_UPGRADE_WAR_GALLEY = BUILDING_DOCK_UPGRADE_WAR_GALLEY,
    TECH_DOCK_UPGRADE_TRIREME = BUILDING_DOCK_UPGRADE_TRIREME,

    // ---------- 攻城工坊：攻城器械升级 ----------
    TECH_SIEGE_UPGRADE_CATAPULT = BUILDING_SIEGE_UPGRADE_CATAPULT,
    TECH_SIEGE_UPGRADE_HEAVYCATAPULT = BUILDING_SIEGE_UPGRADE_HEAVYCATAPULT,
    TECH_SIEGE_UPGRADE_HELEPOLIS = BUILDING_SIEGE_UPGRADE_HELEPOLIS,

    // ---------- 学院：方阵兵升级 ----------
    TECH_COLLAGE_UPGRADE_PHALANX = BUILDING_COLLAGE_UPGRADE_PHALANX,
    TECH_COLLAGE_UPGRADE_CENTURION = BUILDING_COLLAGE_UPGRADE_CENTURION,

    // ---------- 政治机关 ----------
    TECH_GOVERNMENT_ARISTOCRACY = BUILDING_GOVERNMENT_ARISTOCRACY,
    TECH_GOVERNMENT_NOBILITY = BUILDING_GOVERNMENT_NOBILITY,
    TECH_GOVERNMENT_ENGINEERING = BUILDING_GOVERNMENT_ENGINEERING,
    TECH_GOVERNMENT_ALCHEMY = BUILDING_GOVERNMENT_ALCHEMY,
    TECH_GOVERNMENT_BALLISTICS = BUILDING_GOVERNMENT_BALLISTICS,
    TECH_GOVERNMENT_ARCHITECTURE = BUILDING_GOVERNMENT_ARCHITECTURE,
    TECH_GOVERNMENT_LOGISTICS = BUILDING_GOVERNMENT_LOGISTICS,

    // ---------- 神殿：宗教 ----------
    TECH_TEMPLE_POLYTHEISM = BUILDING_TEMPLE_POLYTHEISM,
    TECH_TEMPLE_MYSTICISM = BUILDING_TEMPLE_MYSTICISM,
    TECH_TEMPLE_ASTROLOGY = BUILDING_TEMPLE_ASTROLOGY,
    TECH_TEMPLE_MEDICINE = BUILDING_TEMPLE_MEDICINE,
    TECH_TEMPLE_MONOTHEISM = BUILDING_TEMPLE_MONOTHEISM,
    TECH_TEMPLE_AFTERLIFE = BUILDING_TEMPLE_AFTERLIFE,
    TECH_TEMPLE_FANATICISM = BUILDING_TEMPLE_FANATICISM,
    TECH_TEMPLE_ZEALOTRY = BUILDING_TEMPLE_ZEALOTRY,
    TECH_TEMPLE_SACRIFICE = BUILDING_TEMPLE_SACRIFICE,

    // 位图容量：科技编号必须小于它（128 位 = 两个 U64）。
    // 以后科技超过 128 项时，这个值和 TechnologyBits 的存储要一起扩。
    TECH_MAX_NUM = 128
};

/*==========================================================================
 * 科技位图
 *
 * 科技一共 73 项，编号最大到 97（沿用 BuildingAction 的编号），一个 U64 只有
 * 64 位装不下，所以用「两个 U64」拼成 128 位：low 存 0~63，high 存 64~127。
 * 对外仍然只按科技编号操作，调用方不需要关心落在哪个字上。
 *========================================================================*/
struct TechnologyBits
{
    U64 low;    // 位 0 ~ 63
    U64 high;   // 位 64 ~ 127

    constexpr TechnologyBits() : low(0), high(0) {}
    constexpr TechnologyBits(U64 lowBits, U64 highBits) : low(lowBits), high(highBits) {}

    void add(I32 technology)
    {
        if (technology <= 0 || technology >= TECH_MAX_NUM) {
            return;
        }
        if (technology < 64) {
            low |= (U64(1) << technology);
        } else {
            high |= (U64(1) << (technology - 64));
        }
    }

    void remove(I32 technology)
    {
        if (technology <= 0 || technology >= TECH_MAX_NUM) {
            return;
        }
        if (technology < 64) {
            low &= ~(U64(1) << technology);
        } else {
            high &= ~(U64(1) << (technology - 64));
        }
    }

    [[nodiscard]] bool has(I32 technology) const
    {
        if (technology <= 0 || technology >= TECH_MAX_NUM) {
            return false;
        }
        if (technology < 64) {
            return (low & (U64(1) << technology)) != 0;
        }
        return (high & (U64(1) << (technology - 64))) != 0;
    }

    // this 是否包含 other 里的全部位（other 为空视为满足）
    [[nodiscard]] bool contains(const TechnologyBits& other) const
    {
        return (low & other.low) == other.low && (high & other.high) == other.high;
    }

    [[nodiscard]] bool empty() const
    {
        return low == 0 && high == 0;
    }
};

// 把科技编号转成「只有这一位」的位图。建表时用 T(TECH_XXX) 写前置科技。
// 越界（<=0 或 >=128）返回空位图，避免移位溢出。
constexpr TechnologyBits techBit(I32 technology)
{
    return (technology <= 0 || technology >= TECH_MAX_NUM)
        ? TechnologyBits()
        : (technology < 64
            ? TechnologyBits(U64(1) << technology, 0)
            : TechnologyBits(0, U64(1) << (technology - 64)));
}

// 多个前置科技用 | 串起来：T(A) | T(B)
constexpr TechnologyBits operator|(TechnologyBits lhs, TechnologyBits rhs)
{
    return TechnologyBits(lhs.low | rhs.low, lhs.high | rhs.high);
}

class Development
{
public:
    // 构造函数：civilization 为文明类型（共 16 个，见 Civilization.h）
    Development(WeakPtr<Player> player, I32 civilization);
    ~Development() = default;

    // ---- 科技 ----
    // 某项科技是否已经完成（参数用 Technology 枚举）
    [[nodiscard]] bool TechnologyFinish(I32 technology) const;

    // 记录一项科技完成。action 是 BuildingAction 的值。
    // 返回 true 表示这是一项新科技；重复记录、非科技动作、被本文明封锁的科技都返回 false。
    bool markTechnologyFinish(I32 action);

    // 某项科技是否被本文明的科技掩码封锁（该文明没有这项科技）
    [[nodiscard]] bool TechnologyBlocked(I32 technology) const;

    // 当前时代（CIVILIZATION_STONEAGE / TOOLAGE / BRONZEAGE / IRONAGE）
    [[nodiscard]] I32 getStage() const;
    [[nodiscard]] I32 getCivilization() const;

    // ---- 加成查询 ----
    // 返回某个 sort 下具体 type 的速度加成比例
    [[nodiscard]] Double get_rate_Move(I32 sort, I32 type) const;

    // 返回某个 sort 下具体 type 的血量加成比例
    [[nodiscard]] Double get_rate_Blood(I32 sort, I32 type) const;

    // 返回某个 sort 下具体 type 的额外血量加成
    [[nodiscard]] I32 get_addition_Blood(I32 sort, I32 type) const;

    // 返回某个 sort 下具体 type 的攻击加成比例（armyClass：骑兵/弓兵等）
    [[nodiscard]] Double get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;

    // 返回某个 sort 下具体 type 的攻击数值加成
    [[nodiscard]] I32 get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;

    // 返回某个 sort 下具体 type 的攻击距离加成
    [[nodiscard]] I32 get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;

    // 返回某个 sort 下具体 type 的防御比例加成（attackType_got：受到攻击的类型）
    [[nodiscard]] Double get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const;

    // 返回某个 sort 下具体 type 的防御数值加成
    [[nodiscard]] I32 get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const;

    // 资源可携带的额外数值加成
    [[nodiscard]] I32 get_addition_ResourceCarry(I32 resourceSort) const;

    // 资源采摘速度额外加成比例
    [[nodiscard]] Double get_rate_ResourceGather(I32 resourceSort) const;

    // 资源型建筑额外数量加成
    [[nodiscard]] I32 get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const;

    // 建筑视野加成（城防：全部建筑视野+3）
    [[nodiscard]] I32 get_addition_BuildingVision() const;

    // ---- 时代 ----
    // 升级时代，自动对时代升级 +1
    void stageChange();

    // 是否满足时代升级的条件
    [[nodiscard]] bool stageCanChange() const;

    // ---- 建筑 ----
    // 是否允许建筑进行某个动作（参数为当前资源量）
    [[nodiscard]] bool buildingActionAble(I32 buildingType, I32 actNum,
                                          I32 wood, I32 food, I32 stone, I32 gold) const;

    // 建筑进行某个行动的耗时，返回帧数（非物理时间）
    [[nodiscard]] I32 buildingActionTimeCost(I32 buildingType, I32 actNum) const;

    // 判断建筑是否允许建造（需结合文明、阶段）
    [[nodiscard]] bool buildingCanBuild(I32 buildingType) const;

    // 建造某个建筑的耗时，返回帧数
    [[nodiscard]] I32 buildingBuildTimeCost(I32 buildingType) const;

    // 每次完整运行完一次内核后执行
    void tick();

private:
    // 本玩家是否已经建成某个类型的建筑（建筑表 / 动作表判断前置建筑时用）
    [[nodiscard]] bool hasConstructedBuilding(I32 type) const;

    WeakPtr<Player> player;    // 只能读 player 内元素，不得修改 player 的任何值
    I32 stage;                 // 当前阶段（石器时代等）
    I32 civilization;          // 文明类型（共 16 个）
    TechnologyBits technology; // 已经完成的科技
    TechnologyBits blocked;    // 本文明被封锁的科技（构造时按文明算好）
    U64 blockedBuilding;       // 本文明不能建造的建筑，按 BUILDING_TYPE 索引
};
