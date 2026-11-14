#include "BuildingResource.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

BuildingResource::BuildingResource(I32 type, IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent):Building(type,blockPos,playerScience,percent){
    sort = SORT_BUILDING_RESOURCE;
    this->type = type;
    this->block_pos = blockPos;
    this->playerScience = playerScience;
    //this->Percent = Percent;该参数意义不明，先不实现
    this->cntLeft= getMaxCnt(playerScience);            //构造时为满资源
    decayRate= Double::Zero();	                        //腐烂速度为0。不会腐烂
    gatherAble= true;	                                //可采集
    //sort 改成 SORT_BUILDING_RESOURCE 之后，按正确的 sort 重算一次
    vision = ObjectBaseValue::getBasicVision(sort,type);
    this->maxBlood = computeFullBlood();
    this->currentBlood = this->maxBlood;
    rebuildCrashBox();
}

Double BuildingResource::getMaxCnt(WeakPtr<Development> playerScience){
    const Double base = ObjectBaseValue::getResourceBasicMaxCnt(SORT_BUILDING_RESOURCE,type);
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){
        return base;
    }
    //科技会给资源型建筑加产量（驯养/犁/灌溉那一类）
    return base + Double(science->get_addition_BuildingResourceMaxCnt(SORT_BUILDING_RESOURCE,type));
}

bool BuildingResource::isGatherAble(){
    if(cntLeft>Double::Zero()&&constructed){
        return true;
    }
    return false;
}