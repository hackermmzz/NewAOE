#include "BuildingResource.h"

BuildingResource::BuildingResource(I32 type, IVec2 blockPos, WeakPtr<Development> playerScience , I32 playerRepresent ,I32 Percent):Building(type,blockPos,playerScience,playerRepresent){
    sort = SORT_BUILDING_RESOURCE;
    this->type = type;
    this->block_pos = blockPos;
    this->playerScience = playerScience;
    this->SN = playerRepresent;
    //this->Percent = Percent;该参数意义不明，先不实现
    this->cntLeft= getMaxCnt(playerScience);            //构造时为满资源
    decayRate= Double::Zero();	                        //腐烂速度为0。不会腐烂
    gatherAble= true;	                                //可采集
}

Double BuildingResource::getMaxCnt(WeakPtr<Development> playerScience){
    //等待Development模块的科技加成实现
}
bool BuildingResource::isGatherAble(){
    if(cntLeft>Double::Zero()&&constructed){
        return true;
    }
    return false;
}