#pragma once

#include "Public/vec/vec.hpp"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Animal.h"
#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Civililation.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Farmer.h"
#include "Server/core/Public/var/Missile.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/StaticResource.h"
#include "Server/core/type.h"
#include <array>
#include "Server/core/BaseObject/CrashBox.h"

using namespace std;

class ObjectBaseValue{
    private:
    //开局初始资源
    constexpr static const I32 BasicStartResource[4]={150,200,150,0};
    //对象基础血量表
    constexpr static const I32 BaseBlood_Building[BUILDING_TYPE_MAXNUM]={0};
    constexpr static const I32 BaseBlood_Army[AT_ARMY_MAX_NUM]={0};
    constexpr static const I32 BaseBlood_Farmer[FARMERTYPE_MAXNUM]={0};
    constexpr static const I32 BaseBlood_Animal[ANIMLA_TYPE_MANNUM]={0};
    constexpr static const I32* BaseBlood[SORT_MAXNUM]={BaseBlood_Building,nullptr,BaseBlood_Farmer,BaseBlood_Animal,nullptr,BaseBlood_Building,BaseBlood_Army};
    //对象名
    string Name_Building[BUILDING_TYPE_MAXNUM]={"房屋","谷仓","市镇中心","仓库","农场","市场","瞭望箭塔","警戒箭塔","防御箭塔","防御炮塔","军营","马厩","靶场","船坞","攻城武器厂","学院","政府中心","世界奇观","神庙","初级城墙","中级城墙","高级城墙"};
    string Name_StaticRes[STATICRES_MAXNUM]={"浆果丛","石矿","金矿","鱼群"};
    string Name_Farmer[FARMERTYPE_MAXNUM]={"农民","小型渔船","大型渔船","小型商船","大型商船","小型运输船","重型运输船"};
    string Name_Animal[ANIMLA_TYPE_MANNUM]={"瞪羚","大象","狮子","鳄鱼"};
    string Name_Missile[NUMBER_MISSILE]={"矛","箭","圆石","巨石","弩炮"};
    string Name_Army[AT_ARMY_MAX_NUM]={"棍棒兵","刀斧兵","短剑兵","阔剑兵","长剑兵","军团步兵",
        "投石兵","弓兵","强弓兵","复合弓兵","弓骑兵","重装弓骑兵","象弓兵","战车弓兵","侦察骑兵",
        "骑兵","重骑兵","铁甲骑兵","骆驼骑兵","战车","刀轮战车","战象","装甲战象","投石车","中型投石车","重型投石车","弩炮","重型弩炮","侦察船",
        "桨帆船","三列桨帆船","无敌战船","火帆船","方阵兵","十夫长","百夫长"};
    string* ObjectName[SORT_MAXNUM]={Name_Building,Name_StaticRes,Name_Farmer,Name_Animal,Name_Missile,Name_Building,Name_Army};
    //基础资源最大值
    enum ObjectSort {
    SORT_BUILDING=0,                //建筑
    SORT_STATICRES,                 //静态资源
    SORT_FARMER,                    //农民
    SORT_ANIMAL,                    //动物
    SORT_MISSILE,                   //投掷物
    SORT_BUILDING_RESOURCE,         //资源型建筑（农田）
    SORT_ARMY,                      //军事人员
    SORT_MAXNUM
};
/*enum ANIMAL {
    ANIMAL_TREE = 0,        // 树
    ANIMAL_GAZELLE,         // 瞪羚
    ANIMAL_ELEPHANT,        // 大象
    ANIMAL_LION,            // 狮子
    ANIMAL_ALLIGATOR,       // 鳄鱼
    ANIMLA_TYPE_MANNUM
};*/

    public:
    //获取开局初始资源
    static std::array<I32, 4> getStartResource(I32 civilization){
        array<I32, 4> tmp;
        for(I32 i=0;i<4;i++){tmp[i] =BasicStartResource[i];}
        if(civilization==CIVILIZATION_SHANG)tmp[0]-=40;
        if(civilization==CIVILIZATION_PALMYRENEAN)tmp[0]+=75;
        if(civilization==CIVILIZATION_CARTHAGINIAN){for(int i=0;i<4;i++){tmp[i]+=50;}};
        return tmp;
    }
    //获取对象基础血量
    static Double getBasicBlood(I32 sort,I32 type){
        if(BaseBlood[sort])return BaseBlood[sort][type];
    }
    //对象名字
    std::string getObjectName(I32 sort,I32 type){
        return ObjectName[sort][type];
    }
    //基础资源最大值
    Double getResourceBasicMaxCnt(I32 sort,I32 type){
        switch(sort){
        case SORT_STATICRES:{
            switch (type){
            case STATICRES_Bush:return 150;
            case STATICRES_Stone:return 400;
            case STATICRES_Gold:return 450;
            case STATICRES_Fish:return 250;
            default:return Double::Zero();
        }
        }
        case SORT_ANIMAL:{
            switch (type){
            case ANIMAL_TREE:return 75;
            case ANIMAL_GAZELLE:return 150;
            case ANIMAL_ELEPHANT:return 300;
            case ANIMAL_LION:return 150;
            case ANIMAL_ALLIGATOR:return 150;
            default:return Double::Zero();
            }
        }
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE:{
            switch (type){
            case BUILDING_FARM:return 150;
            default:return Double::Zero();
            }
        }
        default:return Double::Zero();
        }
    }
    //资源腐烂速度
    Double getResourceDecayRate(I32 sort,I32 type){
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
    //基础视野
    I32 getBasicVision(I32 sort,I32 type){

    }
    //基础移动速度
    Double getBasicSpeed(I32 sort,I32 type){

    }
    //农民采集最大量
    Double getFarmerBasicCollectMax(I32 sort,I32 type){
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
    Double getFarmerBasicCollectSpeed(I32 sort,I32 type){
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
    //基础防御力
    std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){//first=近战,second=远程

    }
    //基础攻击力
    std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){//first=近战,second=远程

    }
    //基础攻击范围
    std::pair<I32,I32> getBasicAttackRange(I32 sort,I32 type){//first=范围伤害影响的范围,second=攻击范围

    }
    //基础攻击间隔
    I32 getAttackInterval(I32 sort,I32 type){

    }
    pair<I32,I32> getCrashBox(I32 sort,I32 type){//first=碰撞盒类型,second=碰撞盒尺寸
        
    }
};