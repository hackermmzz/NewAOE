#pragma once

#include "Public/vec/vec.hpp"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Animal.h"
#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Farmer.h"
#include "Server/core/Public/var/Missile.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/StaticResource.h"
#include "Server/core/type.h"
#include "Server/core/BaseObject/CrashBox.h"
#include <array>
#include <string>
#include <utility>

using namespace std;

// 基础数值表。数据来源：《帝国时代：决定版》全面数据表（单位数据 / 建筑数据 /
// 资源与野生动物 / 文明加成与开局 / 说明与来源）。
//
// 单位约定：
//   - 速度单位为「细节坐标 / 秒」，即 区块/秒 × BLOCKSIDELENGTH。表中的 1.1 区块/秒
//     在这里存成 1100。按帧使用时请再除以 GAME_TPS。
//   - 攻击间隔单位为「帧」，由表中的秒数 × GAME_TPS 四舍五入得到。
//   - 攻击距离 / 范围伤害半径单位为「区块」，与表中的射程列一致。
//   - 碰撞盒尺寸单位为「细节坐标」。
//
// 数据表未给出视野的近战单位与船只统一先填 5，后续有实测数据再逐项替换；
// 其余未给出的项填 0，并在对应位置用 TODO 注明，不做推测填值。
class ObjectBaseValue{
    private:
    //开局初始资源。顺序为木材、粮食、石头、黄金。
    //标准开局：食物 200 / 木材 200 / 石料 150 / 黄金 0。
    constexpr static const I32 BasicStartResource[4]={200,200,150,0};
    //对象基础血量表
    constexpr static const I32 BaseBlood_Building[BUILDING_TYPE_MAXNUM]={
        75,     // 房屋
        350,    // 谷仓
        600,    // 市镇中心
        350,    // 仓库
        50,     // 农场
        350,    // 市场
        125,    // 瞭望箭塔
        185,    // 警戒箭塔
        240,    // 防御箭塔
        240,    // 防御炮塔
        350,    // 军营
        350,    // 马厩
        350,    // 靶场
        350,    // 船坞
        350,    // 攻城武器厂
        350,    // 学院
        350,    // 政府中心
        500,    // 世界奇观
        350,    // 神庙
        200,    // 初级城墙
        300,    // 中级城墙
        400     // 高级城墙
    };
    constexpr static const I32 BaseBlood_Army[AT_ARMY_MAX_NUM]={
        40,     // 棍棒兵
        50,     // 刀斧兵
        60,     // 短剑兵
        70,     // 阔剑兵
        80,     // 长剑兵
        140,    // 军团步兵
        25,     // 投石兵
        35,     // 弓兵
        40,     // 强弓兵
        45,     // 复合弓兵
        60,     // 弓骑兵
        90,     // 重装弓骑兵
        600,    // 象弓兵
        70,     // 战车弓箭手
        60,     // 侦察骑兵
        150,    // 骑兵
        150,    // 重骑兵
        240,    // 铁甲骑兵
        125,    // 骆驼骑兵
        100,    // 战车
        120,    // 刀轮战车
        600,    // 战象
        600,    // 装甲战象
        75,     // 投石车
        75,     // 中型投石车
        150,    // 重型投石车
        55,     // 弩炮
        55,     // 重型弩炮
        120,    // 侦察船
        160,    // 桨帆船
        200,    // 三列桨帆船
        135,    // 无敌战船（表中对应「投石三列桨」）
        120,    // 火帆船：数据表未收录，参考桨帆船取值
        120,    // 方阵兵
        120,    // 十夫长
        160,    // 百夫长
        25      // 祭司
    };
    constexpr static const I32 BaseBlood_Farmer[FARMERTYPE_MAXNUM]={
        25,     // 农民
        45,     // 小型渔船
        0,      // 大型渔船：数据表未收录，TODO 待补
        200,    // 小型商船
        0,      // 大型商船：数据表未收录，TODO 待补
        150,    // 小型运输船
        0       // 重型运输船：数据表未收录，TODO 待补
    };
    constexpr static const I32 BaseBlood_Animal[ANIMLA_TYPE_MANNUM]={
        25,     // 树
        8,      // 瞪羚
        45,     // 大象
        20,     // 狮子
        20      // 鳄鱼
    };
    constexpr static const I32* BaseBlood[SORT_MAXNUM]={
        BaseBlood_Building,     // SORT_BUILDING
        nullptr,                // SORT_STATICRES
        BaseBlood_Farmer,       // SORT_FARMER
        BaseBlood_Animal,       // SORT_ANIMAL
        nullptr,                // SORT_MISSILE
        BaseBlood_Building,     // SORT_BUILDING_RESOURCE
        BaseBlood_Army          // SORT_ARMY
    };
    //对象名
    constexpr static const char* const Name_Building[BUILDING_TYPE_MAXNUM]={"房屋","谷仓","市镇中心","仓库","农场","市场","瞭望箭塔","警戒箭塔","防御箭塔","防御炮塔","军营","马厩","靶场","船坞","攻城武器厂","学院","政府中心","世界奇观","神庙","初级城墙","中级城墙","高级城墙"};
    constexpr static const char* const Name_StaticRes[STATICRES_MAXNUM]={"浆果丛","石矿","金矿","鱼群"};
    constexpr static const char* const Name_Farmer[FARMERTYPE_MAXNUM]={"农民","小型渔船","大型渔船","小型商船","大型商船","小型运输船","重型运输船"};
    constexpr static const char* const Name_Animal[ANIMLA_TYPE_MANNUM]={"树","瞪羚","大象","狮子","鳄鱼"};
    constexpr static const char* const Name_Missile[NUMBER_MISSILE]={"矛","箭","圆石","巨石","弩炮"};
    constexpr static const char* const Name_Army[AT_ARMY_MAX_NUM]={"棍棒兵","刀斧兵","短剑兵","阔剑兵","长剑兵","军团步兵",
        "投石兵","弓兵","强弓兵","复合弓兵","弓骑兵","重装弓骑兵","象弓兵","战车弓兵","侦察骑兵",
        "骑兵","重骑兵","铁甲骑兵","骆驼骑兵","战车","刀轮战车","战象","装甲战象","投石车","中型投石车","重型投石车","弩炮","重型弩炮","侦察船",
        "桨帆船","三列桨帆船","无敌战船","火帆船","方阵兵","十夫长","百夫长","祭司"};
    constexpr static const char* const* const ObjectName[SORT_MAXNUM]={
        Name_Building,          // SORT_BUILDING
        Name_StaticRes,         // SORT_STATICRES
        Name_Farmer,            // SORT_FARMER
        Name_Animal,            // SORT_ANIMAL
        Name_Missile,           // SORT_MISSILE
        Name_Building,          // SORT_BUILDING_RESOURCE
        Name_Army               // SORT_ARMY
    };

    // 可移动对象（兵种 / 农民与船只 / 动物）的非血量数值。
    struct UnitValue{
        I32 speed;          // 移动速度：细节坐标/秒
        I32 vision;         // 视野：表未给出的填 0
        I32 attackClose;    // 近战攻击
        I32 attackShoot;    // 远程攻击
        I32 defenceClose;   // 近战护甲
        I32 defenceShoot;   // 穿刺护甲
        I32 attackRange;    // 攻击距离：区块
        I32 splash;         // 范围伤害半径：区块，0 表示无溅射
        I32 attackTicks;    // 攻击间隔：帧
        I32 crashRadius;    // 圆形碰撞盒半径：细节坐标
    };
    // 下标为 AT_*
    constexpr static const UnitValue ArmyValue[AT_ARMY_MAX_NUM]={
        /* 棍棒兵      */ { 1200,  5,     3,  0,  0,  0,  1, 0,  38, 150},
        /* 刀斧兵      */ { 1200,  5,     5,  0,  0,  0,  1, 0,  38, 150},
        /* 短剑兵      */ { 1200,  5,     7,  0,  1,  0,  1, 0,  38, 150},
        /* 阔剑兵      */ { 1200,  5,     9,  0,  1,  0,  1, 0,  38, 150},
        /* 长剑兵      */ { 1200,  5,    11,  0,  2,  0,  1, 0,  38, 150},
        /* 军团步兵    */ { 1200,  5,    13,  0,  2,  0,  1, 0,  38, 150},
        /* 投石兵      */ { 1200,  6,     0,  2,  0,  3,  4, 0,  50, 150},
        /* 弓兵        */ { 1200,  7,     0,  3,  0,  0,  5, 0,  35, 150},
        /* 强弓兵      */ { 1200,  8,     0,  4,  0,  0,  6, 0,  35, 150},
        /* 复合弓兵    */ { 1200,  9,     0,  5,  0,  0,  7, 0,  35, 150},
        /* 弓骑兵      */ { 2200,  9,     0,  7,  0,  2,  7, 0,  38, 180},
        /* 重装弓骑兵  */ { 2500,  9,     0,  8,  0,  1,  7, 0,  38, 180},
        /* 象弓兵      */ {  900,  9,     0,  5,  0,  0,  7, 0,  38, 250},
        /* 战车弓箭手  */ { 2000,  9,     0,  4,  0,  0,  7, 0,  38, 200},
        /* 侦察骑兵    */ { 2000, 13,     3,  0,  0,  0,  1, 0,  23, 180},
        /* 骑兵        */ { 2000,  5,     8,  0,  0,  0,  1, 0,  33, 180},
        /* 重骑兵      */ { 2000,  5,    10,  0,  2,  2,  1, 0,  33, 180},
        /* 铁甲骑兵    */ { 2000,  5,    12,  0,  5,  3,  1, 0,  33, 180},
        /* 骆驼骑兵    */ { 2000,  5,     6,  0,  0,  0,  1, 0,  33, 180},
        /* 战车        */ { 2000,  5,     7,  0,  0,  0,  1, 0,  35, 200},
        /* 刀轮战车    */ { 2000,  5,     9,  0,  1,  1,  1, 0,  35, 200},
        /* 战象        */ {  900,  5,    15,  0,  0,  0,  1, 1,  25, 250},
        /* 装甲战象    */ {  900,  5,    18,  0,  2,  1,  1, 1,  25, 250},
        /* 投石车      */ {  800, 12,     0, 50,  0,  0, 10, 0, 125, 200},
        /* 中型投石车  */ {  800, 14,     0, 50,  0,  0, 12, 1, 125, 200},
        /* 重型投石车  */ {  800, 15,     0, 60,  0,  0, 13, 1, 125, 200},
        /* 弩炮        */ {  800, 11,     0, 40,  0,  0,  9, 1,  75, 200},
        /* 重型弩炮    */ {  800, 12,     0, 45,  0,  0, 10, 1,  38, 200},
        /* 侦察船      */ { 1800,  7,     0,  5,  0,  0,  5, 0,  35, 200},
        /* 桨帆船      */ { 1800,  8,     0,  8,  0,  0,  6, 0,  38, 200},
        /* 三列桨帆船  */ { 1400, 12,     0, 35,  0,  0, 10, 0, 125, 250},  // 按「巨弩舰」取值
        /* 无敌战船    */ { 1400, 11,     0, 35,  0,  0,  9, 0, 125, 250},  // 按「投石三列桨」取值
        /* 火帆船      */ { 1800,  7,    12,  0,  0,  0,  1, 1,  38, 200},  // 近战范围伤害，数据表未收录，参考同级战船取值
        /* 方阵兵      */ {  900,  5,    17,  0,  5,  0,  1, 0,  38, 150},
        /* 十夫长      */ {  900,  5,    20,  0,  7,  0,  1, 0,  38, 150},
        /* 百夫长      */ {  900,  5,    30,  0,  8,  0,  1, 0,  38, 150},
        /* 祭司        */ {  800, 12,     0,  0,  0,  0, 10, 0,   0, 150}
    };
    // 下标为 FARMERTYPE_*
    constexpr static const UnitValue FarmerValue[FARMERTYPE_MAXNUM]={
        /* 农民        */ { 1100,  4,  3,  0,  0,  0,  1, 0,  38, 150},
        /* 小型渔船    */ { 1450,  5,  0,  0,  0,  0,  0, 0,   0, 200},
        /* 大型渔船    */ { 1450,  5,  0,  0,  0,  0,  0, 0,   0, 200},  // TODO 数据表未收录
        /* 小型商船    */ { 2100,  5,  0,  0,  0,  0,  0, 0,   0, 250},
        /* 大型商船    */ { 2100,  5,  0,  0,  0,  0,  0, 0,   0, 250},  // TODO 数据表未收录
        /* 小型运输船  */ { 1450,  5,  0,  0,  0,  0,  0, 0,   0, 250},
        /* 重型运输船  */ { 1450,  5,  0,  0,  0,  0,  0, 0,   0, 250}   // TODO 数据表未收录
    };
    // 下标为 ANIMAL_*，移速按「村民移速 × 倍率」换算（村民 1.1 区块/秒）
    constexpr static const UnitValue AnimalValue[ANIMLA_TYPE_MANNUM]={
        /* 树          */ {    0,  0,  0,  0,  0,  0,  0, 0,   0,   0},
        /* 瞪羚        */ { 1760,  0,  0,  0,  0,  0,  0, 0,   0, 150},  // 村民 ×1.6
        /* 大象        */ {  770,  0, 10,  0,  0,  0,  1, 0,  38, 250},  // 村民 ×0.7
        /* 狮子        */ { 1210,  0,  2,  0,  0,  0,  1, 0,  38, 150},  // 村民 ×1.1
        /* 鳄鱼        */ {  770,  0,  2,  0,  0,  0,  1, 0,  38, 150}   // 村民 ×0.7
    };

    // 建筑的非血量数值。
    struct BuildingValue{
        I32 vision;     // 视野：表未给出的填 0
        I32 side;       // 矩形碰撞盒边长：细节坐标，TODO 数据表未给出占地，暂按常见尺寸估算
    };
    // 下标为 BUILDING_*
    constexpr static const BuildingValue BuildingBase[BUILDING_TYPE_MAXNUM]={
        /* 房屋        */ { 4, 2000},
        /* 谷仓        */ { 0, 2000},
        /* 市镇中心    */ { 7, 4000},
        /* 仓库        */ { 0, 2000},
        /* 农场        */ { 0, 2000},
        /* 市场        */ { 0, 3000},
        /* 瞭望箭塔    */ { 7, 2000},
        /* 警戒箭塔    */ { 8, 2000},
        /* 防御箭塔    */ { 9, 2000},
        /* 防御炮塔    */ { 9, 2000},
        /* 军营        */ { 0, 3000},
        /* 马厩        */ { 0, 3000},
        /* 靶场        */ { 0, 3000},
        /* 船坞        */ { 0, 3000},
        /* 攻城武器厂  */ { 0, 3000},
        /* 学院        */ { 0, 3000},
        /* 政府中心    */ { 0, 3000},
        /* 世界奇观    */ { 0, 5000},
        /* 神庙        */ { 0, 3000},
        /* 初级城墙    */ { 0, 1000},
        /* 中级城墙    */ { 0, 1000},
        /* 高级城墙    */ { 0, 1000}
    };

    // 枚举越界保护：所有查表都先过一遍，避免下标写错时读到别的对象的数据。
    static bool inRange(I32 value,I32 count){
        return value>=0 && value<count;
    }
    // 某个 sort 下合法 type 的上界，供名称表做越界保护。
    static I32 typeCount(I32 sort){
        switch(sort){
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE: return BUILDING_TYPE_MAXNUM;
        case SORT_STATICRES:         return STATICRES_MAXNUM;
        case SORT_FARMER:            return FARMERTYPE_MAXNUM;
        case SORT_ANIMAL:            return ANIMLA_TYPE_MANNUM;
        case SORT_MISSILE:           return NUMBER_MISSILE;
        case SORT_ARMY:              return AT_ARMY_MAX_NUM;
        default:                     return 0;
        }
    }
    static const BuildingValue* findBuildingValue(I32 type){
        return inRange(type,BUILDING_TYPE_MAXNUM) ? &BuildingBase[type] : nullptr;
    }
    static const UnitValue* findUnitValue(I32 sort,I32 type){
        switch(sort){
        case SORT_ARMY:
            return inRange(type,AT_ARMY_MAX_NUM) ? &ArmyValue[type] : nullptr;
        case SORT_FARMER:
            return inRange(type,FARMERTYPE_MAXNUM) ? &FarmerValue[type] : nullptr;
        case SORT_ANIMAL:
            return inRange(type,ANIMLA_TYPE_MANNUM) ? &AnimalValue[type] : nullptr;
        default:
            return nullptr;
        }
    }

    public:
    //获取开局初始资源
    static std::array<I32, 4> getStartResource(I32 civilization){
        array<I32, 4> tmp;
        for(I32 i=0;i<4;i++){tmp[i] =BasicStartResource[i];}
        if(civilization==CIVILIZATION_SHANG)tmp[RESOURCESORT_FOOD]-=40;
        if(civilization==CIVILIZATION_PALMYRENEAN)tmp[RESOURCESORT_FOOD]+=75;
        if(civilization==CIVILIZATION_CARTHAGINIAN){for(int i=0;i<4;i++){tmp[i]+=50;}};
        return tmp;
    }
    //获取对象基础血量
    static Double getBasicBlood(I32 sort,I32 type){
        if(!inRange(sort,SORT_MAXNUM))return Double::Zero();
        if(BaseBlood[sort]==nullptr)return Double::Zero();
        if(!inRange(type,typeCount(sort)))return Double::Zero();
        return Double(BaseBlood[sort][type]);
    }
    //对象名字
    static std::string getObjectName(I32 sort, I32 type){
        if(!inRange(sort,SORT_MAXNUM))return std::string();
        if(ObjectName[sort]==nullptr)return std::string();
        if(!inRange(type,typeCount(sort)))return std::string();
        const char* name=ObjectName[sort][type];
        return name==nullptr ? std::string() : std::string(name);
    }
    //对象名字（带归属信息）。目前公有资源与各玩家的同名对象共用同一张名称表，
    //playerID / teamID / SN 先保留不用，后续需要区分阵营配色或唯一命名时在这里展开。
    static std::string getObjectName(I32 playerID, I32 teamID, I32 sort, I32 type, U64 SN){
        (void)playerID;
        (void)teamID;
        (void)SN;
        return getObjectName(sort,type);
    }
    //基础视野
    static I32 getBasicVision(I32 sort,I32 type){
        if(sort==SORT_BUILDING || sort==SORT_BUILDING_RESOURCE){
            const BuildingValue* value=findBuildingValue(type);
            return value==nullptr ? 0 : value->vision;
        }
        const UnitValue* value=findUnitValue(sort,type);
        return value==nullptr ? 0 : value->vision;
    }
    //基础移动速度，单位：细节坐标/秒
    static Double getBasicSpeed(I32 sort,I32 type){
        const UnitValue* value=findUnitValue(sort,type);
        return value==nullptr ? Double::Zero() : Double(value->speed);
    }
    //基础资源最大值
    static Double getResourceBasicMaxCnt(I32 sort,I32 type){
        switch(sort){
        case SORT_STATICRES:{
            switch (type){
            case STATICRES_Bush:return 150;
            case STATICRES_Stone:return 250;
            case STATICRES_Gold:return 400;
            case STATICRES_Fish:return 250;
            default:return Double::Zero();
        }
        }
        case SORT_ANIMAL:{
            switch (type){
            case ANIMAL_TREE:return 75;
            case ANIMAL_GAZELLE:return 150;
            case ANIMAL_ELEPHANT:return 300;
            case ANIMAL_LION:return 100;
            case ANIMAL_ALLIGATOR:return 100;
            default:return Double::Zero();
            }
        }
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE:{
            switch (type){
            case BUILDING_FARM:return 250;
            default:return Double::Zero();
            }
        }
        default:return Double::Zero();
        }
    }
    //资源腐烂速度
    static Double getResourceDecayRate(I32 sort,I32 type){
        switch(sort){
        case SORT_STATICRES:{
            switch (type){
            case STATICRES_Bush:return Double::Zero();
            case STATICRES_Stone:return Double::Zero();
            case STATICRES_Gold:return Double::Zero();
            case STATICRES_Fish:return Double::Zero();
            default:return Double::Zero();
        }
        }
        case SORT_ANIMAL:{
            switch (type){
            case ANIMAL_TREE:return Double::Zero();
            case ANIMAL_GAZELLE:return Double::Zero();
            case ANIMAL_ELEPHANT:return Double::Zero();
            case ANIMAL_LION:return Double::Zero();
            case ANIMAL_ALLIGATOR:return Double::Zero();
            default:return Double::Zero();
            }
        }
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE:{
            switch (type){
            case BUILDING_FARM:return Double::Zero();
            default:return Double::Zero();
            }
        }
        default:return Double::Zero();
        }
    }
    //农民采集最大量
    static Double getFarmerBasicCollectMax(I32 sort,I32 type){
        switch(sort){
        case SORT_STATICRES:{
            switch (type){
            case STATICRES_Bush:return 10;
            case STATICRES_Stone:return 10;
            case STATICRES_Gold:return 10;
            case STATICRES_Fish:return 10;
            default:return Double::Zero();
        }
        }
        case SORT_ANIMAL:{
            switch (type){
            case ANIMAL_TREE:return 10;
            case ANIMAL_GAZELLE:return 10;
            case ANIMAL_ELEPHANT:return 10;
            case ANIMAL_LION:return 10;
            case ANIMAL_ALLIGATOR:return 10;
            default:return Double::Zero();
            }
        }
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE:{
            switch (type){
            case BUILDING_FARM:return 10;
            default:return Double::Zero();
            }
        }
        default:return Double::Zero();
        }
    }
    //基础采集速度
    static Double getFarmerBasicCollectSpeed(I32 sort,I32 type){
        switch(sort){
        case SORT_STATICRES:{
            switch (type){
            case STATICRES_Bush:return Double("0.2");
            case STATICRES_Stone:return Double("0.15");
            case STATICRES_Gold:return Double("0.15");
            case STATICRES_Fish:return Double("0.2");
            default:return Double::Zero();
        }
        }
        case SORT_ANIMAL:{
            switch (type){
            case ANIMAL_TREE:return Double("0.1");
            case ANIMAL_GAZELLE:return Double("0.2");
            case ANIMAL_ELEPHANT:return Double("0.2");
            case ANIMAL_LION:return Double("0.2");
            case ANIMAL_ALLIGATOR:return Double("0.2");
            default:return Double::Zero();
            }
        }
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE:{
            switch (type){
            case BUILDING_FARM:return Double("0.15");
            default:return Double::Zero();
            }
        }
        default:return Double::Zero();
        }
    }
    //基础防御力 first=近战,second=远程
    static std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){
        const UnitValue* value=findUnitValue(sort,type);
        if(value==nullptr)return std::make_pair(Double::Zero(),Double::Zero());
        return std::make_pair(Double(value->defenceClose),Double(value->defenceShoot));
    }
    //基础攻击力 first=近战,second=远程
    static std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){
        const UnitValue* value=findUnitValue(sort,type);
        if(value==nullptr)return std::make_pair(Double::Zero(),Double::Zero());
        return std::make_pair(Double(value->attackClose),Double(value->attackShoot));
    }
    //基础攻击范围 first=范围伤害影响的范围,second=攻击范围（单位：区块）
    static std::pair<I32,I32> getBasicAttackRange(I32 sort,I32 type){
        const UnitValue* value=findUnitValue(sort,type);
        if(value==nullptr)return std::make_pair(0,0);
        return std::make_pair(value->splash,value->attackRange);
    }
    //基础攻击间隔（单位：帧）
    static I32 getAttackInterval(I32 sort,I32 type){
        const UnitValue* value=findUnitValue(sort,type);
        return value==nullptr ? 0 : value->attackTicks;
    }
    //碰撞盒 first=碰撞盒类型(CrashBoxType),second=碰撞盒尺寸（圆形为半径，矩形为边长，单位：细节坐标）
    static pair<I32,I32> getCrashBox(I32 sort,I32 type){
        if(sort==SORT_BUILDING || sort==SORT_BUILDING_RESOURCE){
            const BuildingValue* value=findBuildingValue(type);
            if(value==nullptr)return std::make_pair(static_cast<I32>(BOX_RECT),0);
            return std::make_pair(static_cast<I32>(BOX_RECT),value->side);
        }
        const UnitValue* value=findUnitValue(sort,type);
        if(value==nullptr)return std::make_pair(static_cast<I32>(BOX_CIRCLE),0);
        return std::make_pair(static_cast<I32>(BOX_CIRCLE),value->crashRadius);
    }
    //圆形碰撞盒半径，供 CrashBox_Circle 直接使用
    static I32 getBasicRadius(I32 sort,I32 type){
        return getCrashBox(sort,type).second;
    }
};
