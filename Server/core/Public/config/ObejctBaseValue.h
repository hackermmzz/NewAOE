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
    string Name_Army[]
    /*    Missile_Spear = 0,          //矛
    Missile_Arrow,              //箭
    Missile_Cobblestone,        //圆石
    Missile_Boulders,           //巨石
    Missile_Ballista,           //弩炮
    NUMBER_MISSILE*/
    constexpr static const string* ObjectName[SORT_MAXNUM]

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

    }
    //基础资源最大值
    Double getResourceBasicMaxCnt(I32 sort,I32 type){

    }
    //基础腐烂速度
    Double getResourceBasicDecayRate(I32 sort,I32 type){

    }
    //基础视野
    I32 getBasicVision(I32 sort,I32 type){

    }
    //基础移动速度
    Double getBasicSpeed(I32 sort,I32 type){

    }
    //基础采集最大量
    Double getFarmerBasicCollectMax(I32 sort,I32 type){

    }
    //基础采集速度
    Double getFarmerBasicCollectSpeed(I32 sort,I32 type){

    }
    //基础防御力
    std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){

    }
    //基础攻击力
    std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){

    }
    //基础攻击范围
    std::pair<Double,Double> getBasicAttackRange(I32 sort,I32 type){

    }
    //基础攻击间隔
    I32 getAttackInterval(I32 sort,I32 type){

    }
};