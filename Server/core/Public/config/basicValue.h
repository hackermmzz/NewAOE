#pragma once


#include <vector>
#include <array>
#include <utility>
#include <map>
#include "Public/type/type.h"
#include "Server/conf.h"
#include "Server/core/type.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Sort.h"

//未完成
struct __GameBasicValue{

    //开局初始资源
    static constexpr I32 BasicStartResource[4]={};
    //各单位初始血量
        static constexpr I32 BuildingBasicBlood[15]={};
    static constexpr I32 FarmerBasicBlood[3]={};
    static constexpr I32 AnimalBasicBlood[5]={};
    static constexpr I32 ArmyBasicBlood[15]={};
    static constexpr const I32* BasicBlood[7]={BuildingBasicBlood,nullptr,FarmerBasicBlood,AnimalBasicBlood,nullptr,BuildingBasicBlood,ArmyBasicBlood};

    //基础资源腐烂速度
    static constexpr Double BasicDecayRate = 0;

    //基础资源最大量                          
    static constexpr Double BasicMaxCnt[5] = {};

    //基础采集最大量
    static constexpr Double BasicCollectMax[5][15] = {};

    //基础采集速度
    static constexpr Double BasicCollectSpeed[4] = {};

    //基础攻击力
    static constexpr Double BasicAttackClose[7][15] = {};
    static constexpr Double BasicAttackShoot[7][15] = {};

    //基础攻击范围
    static constexpr Double BasicAttackRangeClose[7][15] = {};
    static constexpr Double BasicAttackRangeShoot[7][15] = {};

    //基础攻击间隔
    static constexpr Double BasicAttackInterval[7][15] = {};

    //基础防御力：近战、远程
    static constexpr Double BasicDefenceClose[7][15] = {};
    static constexpr Double BasicDefenceShoot[7][15] = {};

    //基础移动速度
    static constexpr Double BasicSpeed[7][15] = {};

    //基础视野
    static constexpr I32 BasicVision[7][15] = {};

    /*static constexpr const All_BasicBlood BasicBlood = {};

    static constexpr const Building_BasicBlood& Building() { return BasicBlood.Building; }
    static constexpr const Farmer_BasicBlood&   Farmer()   { return BasicBlood.Farmer; }
    static constexpr const Animal_BasicBlood&   Animal()   { return BasicBlood.Animal; }
    static constexpr const Army_BasicBlood&     Army()     { return BasicBlood.Army; }*/
    //static constexpr const I32* BasicBlood[7]={BuildingBasicBlood,nullptr,FarmerBasicBlood,AnimalBasicBlood,nullptr,BuildingBasicBlood,ArmyBasicBlood};
    
};
