#include "Human.h"

#include <new>

Human::Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience){
    this->sort = sort;
    this->playerScience = playerScience;
    transported = false;

    //先把所有数值清 0：type 要由派生类设置，update() 也在派生类里调
    maxBlood = Double::Zero();
    currentBlood = Double::Zero();
    defenceClose = Double::Zero();
    defenceShoot = Double::Zero();
    attackClose = Double::Zero();
    attackShoot = Double::Zero();
    attackType = ATTACKTYPE_CLOSE;
    range = false;
    rangeEffect = 0;
    minAttackRange = 0;
    attackCircle = 0;
    attackCurrentTick = 0;
    attackInteval = 0;
    speed = Double::Zero();
    vision = 0;

    setPos(detailPos);
    //人和动物都用圆形碰撞盒，大体积的（船、大象）由派生类用更大半径重建
    new (&crashBoxCircle) CrashBox_Circle(detailPos,CRASHBOX_RADIUS);
}

//按当前科技重算血量与移动速度。
//血量要保住"已经损失的伤害量"：受过的伤不会因为上限提高而回血。
void Human::update(){
    SafePtr<Development> sci = playerScience.lock();

    const I32 oldMax = static_cast<I32>(maxBlood);
    I32 lost = oldMax - static_cast<I32>(currentBlood);
    if(lost < 0){
        lost = 0;
    }

    const Double bloodRate = sci ? sci->get_rate_Blood(sort,type) : Double(1);
    const I32 bloodAdd = sci ? sci->get_addition_Blood(sort,type) : 0;
    maxBlood = applyTech(GameValue::getBasicBlood(sort,type),bloodRate,bloodAdd);

    currentBlood = maxBlood - Double(lost);
    if(currentBlood <= Double::Zero()){
        currentBlood = Double::Zero();
    }
    if(currentBlood > maxBlood){
        currentBlood = maxBlood;
    }

    const Double moveRate = sci ? sci->get_rate_Move(sort,type) : Double(1);
    speed = GameValue::getBasicSpeed(sort,type) * moveRate;
}

bool Human::getTransported() const{
    return transported;
}

void Human::setTransported(bool value){
    transported = value;
}
