#pragma once

#include <array>
#include <string>
#include <utility>

#include "Public/type/type.h"
#include "Server/core/type.h"
#include "Server/core/Public/config/Core.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Farmer.h"
#include "Server/core/Public/var/Animal.h"
#include "Server/core/Public/var/StaticResource.h"
#include "Server/core/Public/var/Missile.h"
#include "Server/core/Public/var/Civilization.h"

/*
对象基础数值表（科技加成之前的裸值）。

数据来源：3.0.7h 的 config.json（485 个键），再用旧版 Army.cpp 的 switch(type)
反解出"枚举 -> 配置键"的对应关系，避免照名字猜错（例如 AT_IMPROVED 实际是
"改进弓箭手"120 血，不是战斧兵）。

补充来源：《各单位数据【繁体中文】.xls》。这张表（含 Rise of Rome 单位与英雄）
可信度一般，只用来填 config.json 里没有的项，行尾标了 [参考] 。
两者都没有的项一律留 0，等有可靠数据再补。

量纲换算：
  旧版 1 块 = 35.77708763999664 细节单位，速度单位是"细节单位/帧"；
  新版 1 块 = BLOCKSIDELENGTH = 1000，所以速度乘 1000/35.77708763999664 ≈ 27.95085。
  攻击间隔配置里是秒，这里统一存帧（秒 × GAME_TPS）。
  攻击距离配置里是块，这里统一存细节单位（块 × BLOCKSIDELENGTH）。
  速度存"每帧位移 × 1000"（整数），避免在常量表里写定点数，取用时再用 milliToDouble 转换。

表中 0 的含义：该字段对这个类型不适用（比如建筑的速度、树木的攻击力）。
*/

//圆形碰撞盒半径（细节单位）：人 / 动物 / 投石车等单体
constexpr I32 CRASHBOX_RADIUS = 249;
//船、大象等大体积
constexpr I32 CRASHBOX_RADIUS_BIG = 495;
//近战接战距离（细节单位，半块）：DISTANCE_ATTACK_CLOSE = 17.8885 旧单位
constexpr I32 DISTANCE_ATTACK_CLOSE = 500;
//村民每次采集的携带上限
constexpr I32 FARMER_CARRY_LIMIT = 10;
//村民采集速度（每帧采集量 ×1000）：旧配置 FARMER_GATHERSPEED_* = 0.02
//注意这是"资源/帧"的比例，不参与块长换算，等内核确定采集结算频率后再校准
constexpr I32 FARMER_GATHER_MILLI = 20;
//村民建造速度（每帧推进的进度 ×1000）：旧配置 FARMER_CONSTRUCTSPEED = 0.02
constexpr I32 FARMER_CONSTRUCT_MILLI = 20;
//攻击起手帧：从开始挥击到真正命中的帧数，配置表里没有，先给 8 帧（≈0.32 秒）
constexpr I32 ATTACK_CIRCLE_FRAMES = 8;

//一条基础数值。所有类型共用一张表，用不到的字段填 0。
struct BasicUnitValue{
    I32 blood;       //最大血量（0 表示没有血量，比如浆果丛）
    I32 vision;      //视野（块）
    I32 atkClose;    //近战攻击力
    I32 atkShoot;    //远程攻击力
    I32 defClose;    //近战防御
    I32 defShoot;    //远程防御
    I32 rangeShoot;  //攻击距离（细节单位）：远程单位的射程；近战单位若有额外够到距离
                     //（比如鳄鱼 2 格）也填这里；0 表示贴身，用 DISTANCE_ATTACK_CLOSE
    I32 interval;    //攻击间隔（帧）
    I32 speedMilli;  //移动速度（细节单位/帧 ×1000）
    I32 maxCnt;      //可采集存量（动物尸体 / 树木 / 矿 / 农田）
    I32 timeBuild;   //建造耗时（秒），只对建筑有效
    I32 foundation;  //建筑地基 FOUNDATION_*，非建筑为 0
    I32 radius;      //圆形碰撞盒半径（细节单位），矩形对象填 0
};

struct __GameBasicValue{

    //----------------------------- 开局资源 -----------------------------
    //木、食、石、金
    static constexpr I32 BasicStartResource[4]={200,200,150,0};

    //特色文明的初始资源替换：{文明, {资源类型, 数量}}
    static constexpr std::pair<I32,std::pair<I32,I32>> FeatureStartResource[2]={
        {CIVILIZATION_SHANG,      {RESOURCESORT_FOOD,160}},
        {CIVILIZATION_PALMYRENEAN,{RESOURCESORT_FOOD,275}},
    };

    //----------------------------- 军队 -----------------------------
    //下标就是 ARMYTYPE
    static constexpr BasicUnitValue ArmyBasic[AT_ARMY_MAX_NUM]={
    //   血  视野 近攻 远攻 近防 远防  射程  间隔   速度   存量 建造 地基 半径
        {   40,  4,   3,   0, 0, 0,     0,  38,  68182,    0,  0, 0, 249 },  // AT_CLUBMAN          棍棒兵
        {   25,  5,   0,   2, 0, 2,  4000,  38,  68182,    0,  0, 0, 249 },  // AT_SLINGER          投石兵
        {   35,  7,   0,   3, 0, 0,  5000,  35,  68182,    0,  0, 0, 249 },  // AT_BOWMAN           弓箭手
        {   60,  8,   3,   0, 0, 0,     0,  23, 113636,    0,  0, 0, 249 },  // AT_SCOUT            侦察骑兵
        {  150,  4,   9,   0, 1, 0,     0,  38,  68182,    0,  0, 0, 249 },  // AT_SWORDSMAN        短剑兵
        {  120,  8,   0,   8, 0, 0,  6000,  35,  68182,    0,  0, 0, 249 },  // AT_IMPROVED         改进弓箭手
        {  150,  4,   8,   0, 0, 0,     0,  33, 113636,    0,  0, 0, 249 },  // AT_CAVALRY          骑兵
        {  300,  4,   0,  20, 0, 0, 10000,  35, 113636,    0,  0, 0, 495 },  // AT_SHIP             战船
        {   75, 13,   0,  50, 0, 0, 10000, 125,  45455,    0,  0, 0, 249 },  // AT_STONE_THROWER    投石车
        {  100, 12,   3,   0, 0, 0, 12000,  38,  56818,    0,  0, 0, 249 },  // AT_PRIEST           祭司
        {  120,  4,  17,   0, 5, 1,     0,  38,  51136,    0,  0, 0, 249 },  // AT_HOPLITE          方阵兵
        {  100,  4,   7,   0, 0, 0,     0,  35, 113636,    0,  0, 0, 249 },  // AT_CHARIOT          四马战车
        {   70,  9,   0,   4, 0, 0,  7000,  38, 113636,    0,  0, 0, 249 },  // AT_CHARIOT_ARCHER   战车弓兵
        {   70,  4,   9,   0, 1, 0,     0,  38,  68182,    0,  0, 0, 249 },  // AT_BROADSWORDSMAN   阔剑兵
        {   45,  9,   0,   5, 0, 0,  7000,  35,  68182,    0,  0, 0, 249 },  // AT_COMPOSITE_BOWMAN 复合弓兵
    };

    //----------------------------- 建筑 -----------------------------
    //下标就是 BUILDING_TYPE；BUILDING_RESOURCE（农田）也复用这张表
    static constexpr BasicUnitValue BuildingBasic[BUILDING_TYPE_MAXNUM]={
    //   血  视野 近攻 远攻 近防 远防  射程  间隔   速度   存量 建造 地基 半径
        {   75,  4,   0,   0, 0, 0,     0,   0,      0,    0, 20, FOUNDATION_MIDDLE, 0 },  // BUILDING_HOME        房屋
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 30, FOUNDATION_BIG,    0 },  // BUILDING_GRANARY     谷仓
        {  600,  4,   0,   0, 0, 0,     0,   0,      0,    0, 60, FOUNDATION_BIG,    0 },  // BUILDING_CENTER      市镇中心
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 30, FOUNDATION_BIG,    0 },  // BUILDING_STOCK       仓库
        {   50,  4,   0,   0, 0, 0,     0,   0,      0,  250, 30, FOUNDATION_BIG,    0 },  // BUILDING_FARM        农田
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 40, FOUNDATION_BIG,    0 },  // BUILDING_MARKET      市场
        {  125, 10,   0,   3, 0, 2,  5000,  38,      0,    0, 80, FOUNDATION_MIDDLE, 0 },  // BUILDING_ARROWTOWER  箭塔
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 30, FOUNDATION_BIG,    0 },  // BUILDING_ARMYCAMP    兵营
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 40, FOUNDATION_BIG,    0 },  // BUILDING_STABLE      马厩
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 40, FOUNDATION_BIG,    0 },  // BUILDING_RANGE       靶场
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 40, FOUNDATION_BIG,    0 },  // BUILDING_DOCK        船坞
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 40, FOUNDATION_BIG,    0 },  // BUILDING_SIEGE       武器工程厂
        {  350,  4,   0,   0, 0, 0,     0,   0,      0,    0, 40, FOUNDATION_BIG,    0 },  // BUILDING_COLLAGE     学院
        {  200,  4,   0,   0, 0, 0,     0,   0,      0,    0,  0, FOUNDATION_BIG,    0 },  // BUILDING_TEMPLE      神庙（未启用，建造时间待定）
        {  200,  4,   0,   0, 0, 0,     0,   0,      0,    0, 10, FOUNDATION_SMALL,  0 },  // BUILDING_WALL        城墙
    };

    //----------------------------- 村民 -----------------------------
    //下标就是 FarmerType：农民 / 渔船 / 运输船
    //农民的数值来自 config.json；两种船 config 里没有血量/速度，标 [参考] 的取自《各单位数据》表
    static constexpr BasicUnitValue FarmerBasic[3]={
    //   血  视野 近攻 远攻 近防 远防  射程  间隔   速度   存量 建造 地基 半径
        {   25,  4,   3,   0, 0, 0,     0,   0,  62500,    0,  0, 0, 249 },  // FARMERTYPE_FARMER     农民
        {   75,  6,   0,   0, 0, 0,     0,   0, 113636,    0,  0, 0, 249 },  // FARMERTYPE_WOOD_BOAT  渔船 [参考]
        {  150,  4,   0,   0, 0, 0,     0,   0,  79545,    0,  0, 0, 249 },  // FARMERTYPE_SAILING    运输船 [参考]
    };

    //----------------------------- 动物 -----------------------------
    //下标就是 ANIMAL；树也在枚举里，走同一张表
    //树/瞪羚/大象/狮子来自 config.json；两种鳄鱼 config 里没有，取自《各单位数据》表 [参考]
    static constexpr BasicUnitValue AnimalBasic[6]={
    //   血  视野 近攻 远攻 近防 远防  射程  间隔   速度   存量 建造 地基 半径
        {   25,  0,   0,   0, 0, 0,     0,   0,  41667,   75,  0, 0, 249 },  // ANIMAL_TREE           树
        {    8,  2,   0,   0, 0, 0,     0,   0,  41667,  150,  0, 0, 249 },  // ANIMAL_GAZELLE        瞪羚
        {   45,  4,  10,   0, 0, 0,     0,   0,  37879,  300,  0, 0, 495 },  // ANIMAL_ELEPHANT       大象
        {   20,  3,   2,   0, 0, 0,     0,   0,  41667,  100,  0, 0, 249 },  // ANIMAL_LION           狮子
        {   20,  5,   4,   0, 0, 0,  2000, 100,  28409,    0,  0, 0, 249 },  // ANIMAL_ALLIGATOR      鳄鱼 [参考]
        {  100,  8,  25,   0, 0, 0,     0,  50,  85227,    0,  0, 0, 495 },  // ANIMAL_ALLIGATOR_KING 鳄鱼王 [参考]
    };

    //----------------------------- 静态资源 -----------------------------
    //下标就是 STATICRES_RESOURCE；这些没有血量，不可被攻击
    static constexpr BasicUnitValue StaticResBasic[4]={
    //   血  视野 近攻 远攻 近防 远防  射程  间隔   速度   存量 建造 地基 半径
        {    0,  0,   0,   0, 0, 0,     0,   0,      0,  150,  0, 0, 0 },  // STATICRES_Bush  浆果丛
        {    0,  0,   0,   0, 0, 0,     0,   0,      0,  250,  0, 0, 0 },  // STATICRES_Stone 石堆
        {    0,  0,   0,   0, 0, 0,     0,   0,      0,  400,  0, 0, 0 },  // STATICRES_Gold  金矿
        {    0,  0,   0,   0, 0, 0,     0,   0,      0,  200,  0, 0, 0 },  // STATICRES_Fish  鱼群
    };

    //----------------------------- 投掷物 -----------------------------
    //投掷物没有血量、视野、防御，速度和存亡由内核控制，先全部占位
    static constexpr BasicUnitValue MissileBasic[NUMBER_MISSILE]={
        {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // Missile_Spear
        {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // Missile_Arrow
        {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // Missile_Cobblestone
        {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // Missile_Boulders
    };

    //按 ObjectSort 索引的表指针。SORT_BUILDING_RESOURCE（农田）复用建筑表。
    static constexpr const BasicUnitValue* BasicTable[7]={
        BuildingBasic,   // SORT_BUILDING
        StaticResBasic,  // SORT_STATICRES
        FarmerBasic,     // SORT_FARMER
        AnimalBasic,     // SORT_ANIMAL
        MissileBasic,    // SORT_MISSILE
        BuildingBasic,   // SORT_BUILDING_RESOURCE
        ArmyBasic,       // SORT_ARMY
    };

    //每张表的大小，用于越界检查（Farmer/Animal/StaticRes/Missile 的枚举没有 MAX 哨兵）
    static constexpr I32 BasicTableSize[7]={
        BUILDING_TYPE_MAXNUM,   // SORT_BUILDING
        4,                      // SORT_STATICRES
        3,                      // SORT_FARMER
        6,                      // SORT_ANIMAL
        NUMBER_MISSILE,         // SORT_MISSILE
        BUILDING_TYPE_MAXNUM,   // SORT_BUILDING_RESOURCE
        AT_ARMY_MAX_NUM,        // SORT_ARMY
    };

    //----------------------------- 显示名 -----------------------------
    static constexpr const char* ArmyName[AT_ARMY_MAX_NUM]={
        "棍棒兵","投石兵","弓箭手","侦察骑兵","短剑兵","改进弓箭手","骑兵","战船",
        "投石车","祭司","方阵兵","四马战车","战车弓兵","阔剑兵","复合弓兵",
    };
    static constexpr const char* BuildingName[BUILDING_TYPE_MAXNUM]={
        "房屋","谷仓","市镇中心","仓库","农田","市场","箭塔","兵营","马厩","靶场",
        "船坞","武器工程厂","学院","神庙","城墙",
    };
    static constexpr const char* FarmerName[3]={"农民","渔船","运输船"};
    static constexpr const char* AnimalName[6]={"树","瞪羚","大象","狮子","鳄鱼","鳄鱼王"};
    static constexpr const char* StaticResName[4]={"浆果丛","石堆","金矿","鱼群"};
};

//"×1000 的整数"转回 Double（每帧位移、采集速度这类）
//不能写成 Double(milli)/Double(1000)：定点除法的中间结果会左移 32 位溢出
FORCE_INLINE Double milliToDouble(I32 milli){
    constexpr I64 kOne=I64{1}<<32;
    return Double::FromRaw((static_cast<I64>(milli)*kOne)/1000);
}

//基础值套用科技：最终值 = 基础值 × 倍率 + 附加值
//倍率 1 表示无加成；附加值是可正可负的整数
FORCE_INLINE Double applyTech(Double base,Double rate,I32 addition){
    return base*rate+Double(addition);
}

//投掷物飞行速度（细节单位/帧）：配置表里没有，先给个经验值，等内核调弹道时再改
constexpr I32 MISSILE_SPEED_MILLI = 800000;
