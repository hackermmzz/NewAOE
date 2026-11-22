#include "BuildingResource.h"
#include "Server/core/Public/config/ObjectBaseValue.h"

BuildingResource::BuildingResource(I32 type, IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent):Building(type,blockPos,playerScience,percent){
    //sort 要在 Building 构造之后才改（建筑编号与资源型建筑共用同一张表，
    //但查询加成时按 sort 区分）。type/block_pos/playerScience 已经由 Building 构造设好。
    sort = SORT_BUILDING_RESOURCE;
    this->cntLeft= getMaxCnt(playerScience);            //构造时为满资源
    decayRate= Double::Zero();	                        //腐烂速度为0。不会腐烂
    gatherAble= true;	                                //可采集
    rebuildCrashBox();
}

Double BuildingResource::getMaxCnt(WeakPtr<Development> playerScience){
    const Double base = ObjectBaseValue::getResourceBasicMaxCnt(SORT_BUILDING_RESOURCE,type);
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr)return base;
    //科技会给资源型建筑加产量（驯养/犁/灌溉那一类）
    return base + Double(science->get_addition_BuildingResourceMaxCnt(SORT_BUILDING_RESOURCE,type));
}

bool BuildingResource::isGatherAble(){
    //没建成之前不能采（结算进度用 Percent，不依赖 isConstructed() 的惰性标志）
    if(!isConstructed())return false;
    return gatherAble;
}