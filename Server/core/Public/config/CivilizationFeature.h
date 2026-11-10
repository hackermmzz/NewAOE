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
    std::string getObjectName(I32 playerID,I32 teamID,I32 sort,I32 type,U64 SN){
        //return "Player："+std::to_string(plarerID)+"队伍："+std::to_string(teamID)+"类型"+std::to_string(sort)+"对象："+std::to_string(type)+"SN"+std::to_string(SN);
    }
    //基础资源最大值
    Double getResourceBasicMaxCnt(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicMaxCnt[sort]);
    }
    //基础腐烂速度
    Double getResourceBasicDecayRate(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicDecayRate);
    }
    //基础视野
    I32 getBasicVision(I32 sort,I32 type){
        return __GameBasicValue::BasicVision[sort][type];
    }
    //基础移动速度
    Double getBasicSpeed(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicSpeed[sort][type]);
    }
    //基础采集最大量
    Double getFarmerBasicCollectMax(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicCollectMax[sort][type]);
    }
    //基础采集速度
    Double getFarmerBasicCollectSpeed(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicCollectSpeed[sort]);
    }
    //基础防御力
    std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){
        return {Double(__GameBasicValue::BasicDefenceClose[sort][type]),Double(__GameBasicValue::BasicDefenceShoot[sort][type])};
    }
    //基础攻击力
    std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){
        return {Double(__GameBasicValue::BasicAttackClose[sort][type]),Double(__GameBasicValue::BasicAttackShoot[sort][type])};
    }
    //基础攻击范围
    std::pair<Double,Double> getBasicAttackRange(I32 sort,I32 type){
        return {Double(__GameBasicValue::BasicAttackRangeClose[sort][type]),Double(__GameBasicValue::BasicAttackRangeShoot[sort][type])};
    }
    //基础攻击间隔
    I32 getAttackInterval(I32 sort,I32 type){
        return __GameBasicValue::BasicAttackInterval[sort][type];
    }
};
