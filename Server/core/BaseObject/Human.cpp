#include "Human.h"
#include "Server/core/Public/config/ObjectBaseValue.h"
#include "Server/core/Public/var/Army.h"

#include <utility>

Human::Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience){
    this->sort = sort;
    setPos(detailPos);
    this->playerScience = playerScience;
    transported = false;
}

bool Human::getTransported() const{return transported;}
void Human::setTransported(bool value){this->transported = value;}
Double Human::getAttackBasic(){
    const std::pair<Double,Double> base =ObjectBaseValue::getBasicAttack(getSort(),getType());
    return attackType == ATTACKTYPE_SHOOT ? base.second : base.first;
}
Double Human::getAttackAdditon(){
    const Double current = attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
    return current - getAttackBasic();
}
Double Human::getDefenceCloseBasic(){return ObjectBaseValue::getBasicDefence(getSort(),getType()).first;}
Double Human::getDefenceShootBasic(){return ObjectBaseValue::getBasicDefence(getSort(),getType()).second;}
Double Human::getDefenceCloseAddition(){return defenceClose - getDefenceCloseBasic();}
Double Human::getDefenceShootAddition(){return defenceShoot - getDefenceShootBasic();}

void Human::refresh(){
    refreshBlood();
    refreshDefence();
    refreshSpeed();
    refreshAttack();
}
void Human::refreshBlood(){
    //科技为空时按「基础值」算，不能让单位停在 0 血：
    //否则一出生就被 isDie() 判死，后面的防御/速度/攻击也全都不刷新了。
    Double newmaxBlood=ObjectBaseValue::getBasicBlood(sort,type);
    SafePtr<Development> science = playerScience.lock();
    if(science != nullptr){
        newmaxBlood=newmaxBlood*science->get_rate_Blood(sort,type)+science->get_addition_Blood(sort,type);
    }
    //第一次刷新（构造时 maxBlood/currentBlood 都是 0）直接按满血初始化。
    //这里不能用 isDie() 拦截：新单位出生时血量就是 0，一拦就永远停在 0，
    //后面的 refreshDefence/Speed/Attack 也会跟着被拦掉。
    if(maxBlood<=Double::Zero()){
        maxBlood=newmaxBlood;
        currentBlood=newmaxBlood;
        return;
    }
    //已经被打死的对象不因上限变化而复活
    if(currentBlood<=Double::Zero()){
        maxBlood=newmaxBlood;
        return;
    }
    //上限涨多少、当前血量跟着涨多少，残血单位不会被科技直接奶满
    currentBlood+=newmaxBlood-maxBlood;
    maxBlood=newmaxBlood;
    if(currentBlood>maxBlood)currentBlood=maxBlood;
    if(currentBlood<Double::Zero())currentBlood=Double::Zero();
}
void Human::refreshDefence(){
    if(isDie()){return;}
    const std::pair<Double,Double> base=ObjectBaseValue::getBasicDefence(getSort(),getType());
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){
        //没有科技：直接取基础值
        defenceClose=base.first;
        defenceShoot=base.second;
        return;
    }
    I32 armyClass=-1;
    if(getSort()==SORT_ARMY)switch(type){
        case AT_CLUBMAN:
        case AT_AXEMAN:
        case AT_SHORT_SWORDSMAN:
        case AT_BROAD_SWORDSMAN:
        case AT_LONG_SWORDSMAN:
        case AT_LEGION:
        case AT_HOPLITE:
        case AT_PHALANX:
        case AT_CENTURION:
        {
            armyClass =ARMY_INFANTRY;
            break;
        }
        case AT_SLINGER:
        case AT_BOWMAN:
        case AT_IMPROVED_BOWMAN:
        case AT_COMPOSITE_BOWMAN:
        case AT_HORSE_ARCHER:
        case AT_HEAVY_HORSE_ARCHER:
        case AT_ELEPHANT_ARCHER:
        case AT_CHARIOT_ARCHER:
        {
            armyClass =ARMY_ARCHER;
            break;
        }
        case AT_SCOUT:
        case AT_CAVALRY:
        case AT_HEAVY_CAVALRY:
        case AT_CATAPHRACT:
        case AT_CAMEL_RIDER:
        case AT_CHARIOT:
        case AT_SCYTHECHARIOT:
        case AT_WAR_ELEPHANT:
        case AT_ARMORED_ELEPHANT:
        {
            armyClass =ARMY_RIDER;
            break;
        }
        case AT_PRIEST:
        {
            armyClass =ARMY_FLAMEN;
            break;
        }
        case AT_STONE_THROWER:
        case AT_CATAPULT:
        case AT_HEAVY_CATAPULT:
        case AT_BALLISTA:
        case AT_HELEPOLIS:
        {
            armyClass =ARMY_SIEGE;
            break;
        }
        case AT_SCOUT_SHIP:
        case AT_WAR_GALLEY:
        case AT_TRIREME:
        case AT_CATAPULT_TRIREME:
        case AT_FIRESAILBOAT:
        {
            armyClass =ARMY_SHIP;
            break;
        }
        default:break;
    }
    defenceClose=base.first*science->get_rate_Defence(sort,type,armyClass,ATTACKTYPE_CLOSE)+science->get_addition_Defence(sort,type,armyClass,ATTACKTYPE_CLOSE);
    defenceShoot=base.second*science->get_rate_Defence(sort,type,armyClass,ATTACKTYPE_SHOOT)+science->get_addition_Defence(sort,type,armyClass,ATTACKTYPE_SHOOT);
}
void Human::refreshSpeed(){
    if(isDie()){return;}
    speed = ObjectBaseValue::getBasicSpeed(sort,type);
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){return;}
    speed = speed*science->get_rate_Move(sort,type);
}
void Human::refreshAttack(){
    if(isDie()){return;}
    SafePtr<Development> science = playerScience.lock();
    const std::pair<Double,Double> attackBase =ObjectBaseValue::getBasicAttack(sort,type);
    I32 armyClass=-1;
    if(getSort()==SORT_ARMY)switch(type){
        case AT_CLUBMAN:
        case AT_AXEMAN:
        case AT_SHORT_SWORDSMAN:
        case AT_BROAD_SWORDSMAN:
        case AT_LONG_SWORDSMAN:
        case AT_LEGION:
        case AT_HOPLITE:
        case AT_PHALANX:
        case AT_CENTURION:
        {
            armyClass =ARMY_INFANTRY;
            break;
        }
        case AT_SLINGER:
        case AT_BOWMAN:
        case AT_IMPROVED_BOWMAN:
        case AT_COMPOSITE_BOWMAN:
        case AT_HORSE_ARCHER:
        case AT_HEAVY_HORSE_ARCHER:
        case AT_ELEPHANT_ARCHER:
        case AT_CHARIOT_ARCHER:
        {
            armyClass =ARMY_ARCHER;
            break;
        }
        case AT_SCOUT:
        case AT_CAVALRY:
        case AT_HEAVY_CAVALRY:
        case AT_CATAPHRACT:
        case AT_CAMEL_RIDER:
        case AT_CHARIOT:
        case AT_SCYTHECHARIOT:
        case AT_WAR_ELEPHANT:
        case AT_ARMORED_ELEPHANT:
        {
            armyClass =ARMY_RIDER;
            break;
        }
        case AT_PRIEST:
        {
            armyClass =ARMY_FLAMEN;
            break;
        }
        case AT_STONE_THROWER:
        case AT_CATAPULT:
        case AT_HEAVY_CATAPULT:
        case AT_BALLISTA:
        case AT_HELEPOLIS:
        {
            armyClass =ARMY_SIEGE;
            break;
        }
        case AT_SCOUT_SHIP:
        case AT_WAR_GALLEY:
        case AT_TRIREME:
        case AT_CATAPULT_TRIREME:
        case AT_FIRESAILBOAT:
        {
            armyClass =ARMY_SHIP;
            break;
        }
        default:break;
    }
    if(attackBase.first > Double::Zero()){
        attackClose = science==nullptr ? attackBase.first
                    : attackBase.first*science->get_rate_Attack(sort,type,armyClass,ATTACKTYPE_CLOSE)+ Double(science->get_addition_Attack(sort,type,armyClass,ATTACKTYPE_CLOSE));
    }else{
        attackClose = Double::Zero();
    }
    if(attackBase.second > Double::Zero()){
        attackShoot = science==nullptr ? attackBase.second
                    : attackBase.second * science->get_rate_Attack(sort,type,armyClass,ATTACKTYPE_SHOOT) + Double(science->get_addition_Attack(sort,type,armyClass,ATTACKTYPE_SHOOT));
    }else{
        attackShoot = Double::Zero();
    }
    const std::pair<I32,I32> rangeBase = ObjectBaseValue::getBasicAttackRange(sort,type);
    attackType = attackBase.second > Double::Zero() ? ATTACKTYPE_SHOOT : ATTACKTYPE_CLOSE;
    rangeEffect = rangeBase.first;
    range = rangeBase.first > 0;
    minAttackRange = rangeBase.second;

    I32 interval = 0;
    I32 circle = ATTACK_CIRCLE_FRAMES;
    if(science != nullptr){
        minAttackRange += science->get_addition_DisAttack(sort,type,armyClass,attackType);
        interval = science->get_speed_attack(sort,type,armyClass)[0];
        circle   = science->get_speed_attack(sort,type,armyClass)[1];
    }
    //科技没给间隔就退回数值表；再不行给 1 帧，避免内核做取模/除法时除零
    if(interval<=0){interval=ObjectBaseValue::getAttackInterval(sort,type);}
    if(interval<=0){interval=1;}
    attackInteval=interval;
    //科技没给前摇（Development 目前恒返回 0，表里也没有这一列）就退回默认半秒
    if(circle<=0){circle=ATTACK_CIRCLE_FRAMES;}
    attackCircle=circle;
}
