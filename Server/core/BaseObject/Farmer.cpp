#include "Farmer.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Sort.h"

#include <new>
#include <utility>

Farmer::Farmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type)
    : Human(SORT_FARMER, detailPos, playerScience)
{
    //type 是 FarmerType：农民 / 渔船 / 运输船
    this->type = type;

    resource = 0;
    resourceSort = -1;          //-1 表示空手
    quantity_GatherOnce = 1;    //每次采集搬运 1 点，具体速度见 getFarmerBasicCollectSpeed

    vision = GameValue::getBasicVision(SORT_FARMER,type);
    attackInteval = GameValue::getAttackInterval(SORT_FARMER,type);
    attackType = ATTACKTYPE_CLOSE;
    attackCircle = ATTACK_CIRCLE_FRAMES;

    new (&crashBoxCircle) CrashBox_Circle(detailPos,GameValue::getBasicRadius(SORT_FARMER,type));

    update();
}

//重算攻击/防御/携带上限（血量、速度由 Human::update 处理）
void Farmer::update(){
    Human::update();

    SafePtr<Development> sci = getPlayerScience().lock();

    const std::pair<Double,Double> baseAtk = GameValue::getBasicAttack(sort,type);
    const Double atkRate = sci ? sci->get_rate_Attack(sort,type,ARMY_INFANTRY,attackType) : Double(1);
    const I32 atkAdd = sci ? sci->get_addition_Attack(sort,type,ARMY_INFANTRY,attackType) : 0;
    attackClose = applyTech(baseAtk.first,atkRate,atkAdd);
    attackShoot = applyTech(baseAtk.second,atkRate,atkAdd);

    const std::pair<Double,Double> baseDef = GameValue::getBasicDefence(sort,type);
    const Double dcRate = sci ? sci->get_rate_Defence(sort,type,ARMY_INFANTRY,ATTACKTYPE_CLOSE) : Double(1);
    const I32 dcAdd = sci ? sci->get_addition_Defence(sort,type,ARMY_INFANTRY,ATTACKTYPE_CLOSE) : 0;
    defenceClose = applyTech(baseDef.first,dcRate,dcAdd);

    const Double dsRate = sci ? sci->get_rate_Defence(sort,type,ARMY_INFANTRY,ATTACKTYPE_SHOOT) : Double(1);
    const I32 dsAdd = sci ? sci->get_addition_Defence(sort,type,ARMY_INFANTRY,ATTACKTYPE_SHOOT) : 0;
    defenceShoot = applyTech(baseDef.second,dsRate,dsAdd);

    //携带上限：科技是按资源种类给的，这里按"当前正在采的资源"取，空手时按木材算
    const I32 resourceForCarry = (resourceSort >= 0) ? resourceSort : RESOURCESORT_WOOD;
    I32 carry = static_cast<I32>(GameValue::getFarmerBasicCollectMax(sort,type));
    if(sci){
        carry += sci->get_addition_ResourceCarry(resourceForCarry);
    }
    resourceMax = carry;

    if(resource > resourceMax){
        resource = resourceMax;
    }
}

I32 Farmer::getResourceNowHave(){
    return resource;
}

I32 Farmer::getResourceHaveMax(){
    return resourceMax;
}

I32 Farmer::getResourceHaveAddition(){
    //上限减掉基础值就是科技给的那部分
    return resourceMax - static_cast<I32>(GameValue::getFarmerBasicCollectMax(sort,type));
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
    if(!human){
        return;
    }
    humanTransport.push_back(human);
    human->setTransported(true);
}
