#include "Farmer.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/config/ObjectBaseValue.h"

Farmer::Farmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type): Human(SORT_FARMER, detailPos, playerScience){
    this->type = type;                 //FARMERTYPE_*，具体角色（农民/渔船/运输船…）
    this->resource = 0;
    //刚出生还没有在采任何资源，等采集指令下来再定
    this->resourceSort = -1;
    //基础携带量；实际上限还要加上科技加成，见 getResourceHaveMax()
    this->resourceMax = FARMER_DEFAULT_CARRY;
    //一次能采多少，同样等采集指令下来再定
    this->quantity_GatherOnce = 0;
    vision = ObjectBaseValue::getBasicVision(SORT_FARMER,type);
    name = ObjectBaseValue::getObjectName(SORT_FARMER,type);
    //构造函数里血量/攻防/速度还是空的，先按当前科技填一遍
    Human::refresh();
    rebuildCrashBox();
}

I32 Farmer::getResourceNowHave(){return resource;}
I32 Farmer::getResourceHaveMax(){return resourceMax + getResourceHaveAddition();}
I32 Farmer::getResourceHaveAddition(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr || resourceSort < 0){return 0; }
    return science->get_addition_ResourceCarry(resourceSort);
}
I32 Farmer::getResourceSort(){return resourceSort;}
I32 Farmer::getQuantityGather(){return quantity_GatherOnce;}
bool Farmer::isFullBackpack(){return resource >= getResourceHaveMax();}

void Farmer::addResource(I32 quantity,I32 resourceSort){
    if(resourceSort != this->resourceSort){//换成采集另一种资源了：手里原来的资源直接丢掉，改采新的。
        resource = 0;
        this->resourceSort = resourceSort;
    }
    //上限要按「新的资源类型」算：不同资源的携带加成不一样
    const I32 capacity = getResourceHaveMax();
    resource += quantity;
    if(resource > capacity){resource = capacity;}
}

void Farmer::transportHuman(SafePtr<Human> human){
    humanTransport.push_back(human);
    human->setTransported(true);
}

I32 Farmer::getTransportMax(){
    if(sort == SORT_FARMER){
        switch(type){
            case FARMERTYPE_LIGHT_TRANSPORT:
                return 5;
            case FARMERTYPE_HEAVY_TRANSPORT:
                return 10;
            default:
                return 0;
        }
    }else{
        return 0;
    }
}