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
    static constexpr I32 BasicStartResource[4]={200,200,150,0};
    //各单位初始血量
        static constexpr I32 BuildingBasicBlood[15]={ 
    //  房屋   谷仓  市镇中心  仓库   农场     学院  神庙  城墙
        1000,  1000, 1000,   1000,  1000, 
    //  市场   箭塔   军营   马厩   靶场   
        1000,  1000, 2000,  2000, 2000,
    //  船坞   攻城武器厂 学院   神庙   城墙
        2000,   5000,  5000,  5000, 5000};
    static constexpr I32 FarmerBasicBlood[3]={ 
    //  农民   木筏   帆船
        25,  25 ,   25 };
    static constexpr I32 AnimalBasicBlood[5]={ 
    //  树      瞪羚      大象    狮子    鳄鱼
        100,    100,    100,    100,   100};
    static constexpr I32 ArmyBasicBlood[15]={ 
    //棍棒兵    投石兵  弓箭兵  侦察骑兵    战斧兵或敌方近战单位
        100,  100,     100,  100,   100,   
    //改进型单位或敌方单位   骑兵       战船    投石车      祭司
        100,                100,        200,    200,    200,
    //方阵兵      战车（四马战车）  战车弓箭手  阔剑兵  复合弓兵
        200,        500,            500,        500,    500};
    static constexpr const I32* BasicBlood[7]={BuildingBasicBlood,nullptr,FarmerBasicBlood,AnimalBasicBlood,nullptr,BuildingBasicBlood,ArmyBasicBlood};

    //基础资源腐烂速度
    static constexpr Double BasicDecayRate = 0.4;

    //基础资源最大量                            树      谷物     石头    金子 猎物
    static constexpr Double BasicMaxCnt[5] = {10,       10,     10,     10, 10};

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
