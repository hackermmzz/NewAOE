#include "BuildingResource.h"
#include "Server/core/Public/var/Sort.h"

BuildingResource::BuildingResource(I32 type, IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent)
    :Building(type,blockPos,playerScience,percent){
    sort = SORT_BUILDING_RESOURCE;
    this->type = type;
    this->playerScience = playerScience;

    //资源建筑不吃"建好之后才完成"的科技：存量在这里算一次就定死
    this->cntLeft = getMaxCnt(playerScience);
    this->maxCnt = this->cntLeft;
    decayRate = Double::Zero();     //农田不腐烂
    gatherAble = true;
}

//农田的存量 = 基础存量 + 科技给的额外上限
Double BuildingResource::getMaxCnt(WeakPtr<Development> playerScience){
    const Double base = GameValue::getResourceBasicMaxCnt(sort,type);
    SafePtr<Development> sci = playerScience.lock();
    if(!sci){
        return base;
    }
    return base + Double(sci->get_addition_BuildingResourceMaxCnt(sort,type));
}

bool BuildingResource::isGatherAble(){
    if(cntLeft > Double::Zero() && constructed){
        return true;
    }
    return false;
}
