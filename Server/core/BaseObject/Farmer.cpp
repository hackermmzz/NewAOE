#include "Farmer.h"
#include "Server/core/Public/var/Sort.h"

Farmer::Farmer(IVec2 detailPos , WeakPtr<Development> playerScience , I32 playerRepresent,I32 type)
    : Human(SORT_FARMER, detailPos, playerScience, playerRepresent)
{
    this->resourceSort = type;
    //等待Development处理
    //this->resourceMax =;
    //this->quantity_GatherOnce =;
    
    this->resource = 0;
}

I32 Farmer::getResourceNowHave(){
    return resource;
}

I32 Farmer::getResourceHaveMax(){
    return resourceMax;
}

I32 Farmer::getResourceHaveAddition(){
    //等待Development完成
}

I32 Farmer::getResourceSort(){
    return resourceSort;
}

I32 Farmer::getQuantityGather(){
    return quantity_GatherOnce;
}

bool Farmer::isFullBackpack(){
    return resource >= resourceMax;
}

void Farmer::addResource(I32 quantity,I32 resourceSort){
    if(resourceSort != this->resourceSort){
        return;
    }
    resource += quantity;
    if(resource > resourceMax){
        resource = resourceMax;
    }
}

void Farmer::transportHuman(SafePtr<Human> human){
    humanTransport.emplace_back(human);
}

