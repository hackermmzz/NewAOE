#include "Farmer.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

Farmer::Farmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type)
    : Human(SORT_FARMER, detailPos, playerScience)
{
    this->type = type;                 //FARMERTYPE_*，具体角色（农民/渔船/运输船…）
    this->resource = 0;
    //刚出生还没有在采任何资源，等采集指令下来再定
    this->resourceSort = -1;
    //携带量：基础值 + 科技额外加成（见 getResourceHaveAddition）
    this->resourceMax = FARMER_DEFAULT_CARRY;
    //一次能采多少，同样等采集指令下来再定
    this->quantity_GatherOnce = 0;

    vision = ObjectBaseValue::getBasicVision(SORT_FARMER,type);
    //构造函数里血量/攻防/速度还是空的，先按当前科技填一遍
    refreshByScience();
    rebuildCrashBox();
}

I32 Farmer::getResourceNowHave(){
    return resource;
}

I32 Farmer::getResourceHaveMax(){
    return resourceMax;
}

I32 Farmer::getResourceHaveAddition(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr || resourceSort < 0){
        return 0;
    }
    return science->get_addition_ResourceCarry(resourceSort);
}

I32 Farmer::getResourceSort(){
    return resourceSort;
}

I32 Farmer::getQuantityGather(){
    return quantity_GatherOnce;
}

bool Farmer::isFullBackpack(){
    return resource >= resourceMax + getResourceHaveAddition();
}

void Farmer::addResource(I32 quantity,I32 resourceSort){
    if(resourceSort != this->resourceSort){
        return;
    }
    const I32 capacity = resourceMax + getResourceHaveAddition();
    resource += quantity;
    if(resource > capacity){
        resource = capacity;
    }
}

void Farmer::transportHuman(SafePtr<Human> human){
    humanTransport.push_back(human);
    human->setTransported(true);
}

