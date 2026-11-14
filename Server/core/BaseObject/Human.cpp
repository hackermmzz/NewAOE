#include "Human.h"
#include "Server/core/Public/config/ObejctBaseValue.h"
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
    maxBlood = ObjectBaseValue::getBasicBlood(objectSort,objectType)
             * science->get_rate_Blood(objectSort,objectType)
             + Double(science->get_addition_Blood(objectSort,objectType));
    //加成被降下来时，当前血量不能超过新的上限
    if(currentBlood > maxBlood){
        currentBlood = maxBlood;
    }

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

    //---- 攻击距离 ----
    minAttackRange = ObjectBaseValue::getBasicAttackRange(objectSort,objectType).second
                   + science->get_addition_DisAttack(objectSort,objectType,armyClass,attackType);
}
