#pragma once

#include "basicValue.h"

class GameValue{
    private:
    //特色文明开局资源量
    static constexpr pair<I32,std::pair<I32,I32>> FeatureStartResource[2]={ {CIVILIZATION_SHANG, {RESOURCESORT_FOOD,160} }, {CIVILIZATION_PALMYRENEAN, {RESOURCESORT_FOOD,275} }};

    public:
    //获取开局初始资源
    static std::array<I32, 4> getStartResource(I32 civilization){
        array<I32, 4> tmp;
        for(I32 i=0;i<4;i++){
            tmp[i] = __GameBasicValue::BasicStartResource[i];
        }
        return tmp;
    }
    static Double getBasicBlood(I32 sort,I32 type){//这里不做检查，调用者自己检查，防止指针越界访问
        return Double(__GameBasicValue::BasicBlood[sort][type]);
    }
    //对象名字
    std::string getObjectName(I32 plarerID,I32 teamID,I32 sort,I32 type,U64 SN){
        //return "Player："+std::to_string(plarerID)+"队伍："+std::to_string(teamID)+"类型"+std::to_string(sort)+"对象："+std::to_string(type)+"SN"+std::to_string(SN);
    }
    //基础采集最大值
    Double getResourceBasicMaxCnt(I32 sort,I32 type){
        //return Double(__GameBasicValue::BasicMaxCnt[sort][type]);
    }
    //基础腐烂速度
    Double getResourceBasicDecayRate(I32 sort,I32 type){
        //return Double(__GameBasicValue::BasicRotSpeed[sort][type]);
    }
    //基础视野
    I32 getBasicVision(I32 sort,I32 type){
        //return __GameBasicValue::BasicVision[sort][type];
    }
    //基础移动速度
    Double getBasicSpeed(I32 sort,I32 type){
        //return Double(__GameBasicValue::BasicMoveSpeed[sort][type]);
    }
    //基础最大收集量
    Double getFarmerBasicCollectMax(I32 sort,I32 type){
        //return Double(__GameBasicValue::BasicMaxCollect[sort][type]);
    }
    //基础采集速度
    Double getFarmerBasicCollectSpeed(I32 sort,I32 type){
        //return Double(__GameBasicValue::BasicCollectSpeed[sort][type]);
    }
    //基础防御力
    std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){
        //return {Double(__GameBasicValue::BasicDef
    }
    //基础攻击力
    std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){
        //return {Double(__GameBasicValue::BasicAttack[sort][type].first),Double(__GameBasicValue::BasicAttack[sort][type].second)};
    }
    //基础攻击范围
    std::pair<Double,Double> getBasicAttackRange(I32 sort,I32 type){
        //return {Double(__GameBasicValue::BasicAttackRange[sort][type].first),Double(__GameBasicValue::BasicAttackRange[sort][type].second)};
    }
    //基础攻击间隔
    I32 getAttackInterval(I32 sort,I32 type){
        //return __GameBasicValue::BasicAttackInterval[sort][type];
    }
};
