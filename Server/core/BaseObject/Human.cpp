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

bool Human::getTransported() const{
    return transported;
}

void Human::setTransported(bool value){
    this->transported = value;
}

I32 Human::getArmyClass() const{
    //农民/渔船等经济单位没有兵种大类，按步兵检索科技。
    return ARMY_INFANTRY;
}

//---- UI 用的「基础值 / 加成值」拆分 ----
//基础值一律取 ObejctBaseValue，加成值 = 当前值（已含科技）- 基础值。

Double Human::getAttackBasic(){
    const std::pair<Double,Double> base =
        ObjectBaseValue::getBasicAttack(getSort(),getType());
    return attackType == ATTACKTYPE_SHOOT ? base.second : base.first;
}

Double Human::getAttackAdditon(){
    const Double current = attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
    return current - getAttackBasic();
}

Double Human::getDefenceCloseBasic(){
    return ObjectBaseValue::getBasicDefence(getSort(),getType()).first;
}

Double Human::getDefenceShootBasic(){
    return ObjectBaseValue::getBasicDefence(getSort(),getType()).second;
}

Double Human::getDefenceCloseAddition(){
    return defenceClose - getDefenceCloseBasic();
}

Double Human::getDefenceShootAddition(){
    return defenceShoot - getDefenceShootBasic();
}

// 构造时写进 BloodHaver / Attacker / MoveObject 的都是「基础值」，
// 科技一变这些缓存就失效了，这里统一按「基础值 × 比例加成 + 数值加成」重算一遍。
void Human::refreshByScience(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){
        return;
    }

    const I32 objectSort = getSort();
    const I32 objectType = getType();
    const I32 armyClass = getArmyClass();

    //---- 血量 ----
    //上限涨多少、当前血量跟着涨多少，残血单位不会被科技直接奶满
    const Double oldMaxBlood = maxBlood;
    const Double oldCurrentBlood = currentBlood;
    maxBlood = ObjectBaseValue::getBasicBlood(objectSort,objectType)
             * science->get_rate_Blood(objectSort,objectType)
             + Double(science->get_addition_Blood(objectSort,objectType));
    syncBloodAfterMaxChange(oldMaxBlood,oldCurrentBlood);

    //---- 移动速度 ----
    speed = ObjectBaseValue::getBasicSpeed(objectSort,objectType)
          * science->get_rate_Move(objectSort,objectType);

    //---- 防御（first=近战，second=远程）----
    const std::pair<Double,Double> defenceBase =
        ObjectBaseValue::getBasicDefence(objectSort,objectType);
    defenceClose = defenceBase.first
                 * science->get_rate_Defence(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE)
                 + Double(science->get_addition_Defence(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE));
    defenceShoot = defenceBase.second
                 * science->get_rate_Defence(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT)
                 + Double(science->get_addition_Defence(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT));

    //---- 攻击（first=近战，second=远程）----
    const std::pair<Double,Double> attackBase =
        ObjectBaseValue::getBasicAttack(objectSort,objectType);
    attackClose = attackBase.first
                * science->get_rate_Attack(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE)
                + Double(science->get_addition_Attack(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE));
    attackShoot = attackBase.second
                * science->get_rate_Attack(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT)
                + Double(science->get_addition_Attack(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT));

    //---- 攻击方式、范围伤害与攻击距离 ----
    const std::pair<I32,I32> rangeBase =
        ObjectBaseValue::getBasicAttackRange(objectSort,objectType);
    //有远程伤害的按远程攻击处理，否则近战
    attackType = attackBase.second > Double::Zero() ? ATTACKTYPE_SHOOT : ATTACKTYPE_CLOSE;
    rangeEffect = rangeBase.first;
    range = rangeBase.first > 0;
    minAttackRange = rangeBase.second
                   + science->get_addition_DisAttack(objectSort,objectType,armyClass,attackType);
}
