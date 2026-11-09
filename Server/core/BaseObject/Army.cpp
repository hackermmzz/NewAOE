#include "Army.h"
#include "Server/core/Public/var/Army.h"

#include <new>
#include <utility>

namespace{
//兵种大类：科技加成按大类给（步兵/弓兵/骑兵/祭司/攻城/船）
I32 GetArmyClass(I32 type){
    switch(type){
    case AT_CLUBMAN:
    case AT_SWORDSMAN:
    case AT_HOPLITE:
    case AT_BROADSWORDSMAN:
        return ARMY_INFANTRY;
    case AT_SLINGER:
    case AT_BOWMAN:
    case AT_IMPROVED:
    case AT_CHARIOT_ARCHER:
    case AT_COMPOSITE_BOWMAN:
        return ARMY_ARCHER;
    case AT_SCOUT:
    case AT_CAVALRY:
    case AT_CHARIOT:
        return ARMY_RIDER;
    case AT_PRIEST:
        return ARMY_FLAMEN;
    case AT_STONE_THROWER:
        return ARMY_SIEGE;
    case AT_SHIP:
        return ARMY_SHIP;
    default:
        return ARMY_INFANTRY;
    }
}
}

Army::Army(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience)
    : Human(SORT_ARMY, detailPos, playerScience)
{
    //type 存具体兵种：getName()、查表、科技判断都要用它，不能覆盖成大类
    this->type = type;
    this->armyClass = GetArmyClass(type);

    const std::pair<Double,Double> atk = GameValue::getBasicAttack(SORT_ARMY,type);
    if(type == AT_PRIEST){
        attackType = ATTACKTYPE_CHANGE;     //祭司不造成伤害，攻击类型单独标出来
    }else if(atk.second > Double::Zero()){
        attackType = ATTACKTYPE_SHOOT;
    }else{
        attackType = ATTACKTYPE_CLOSE;
    }

    vision = GameValue::getBasicVision(SORT_ARMY,type);
    attackInteval = GameValue::getAttackInterval(SORT_ARMY,type);
    attackCircle = ATTACK_CIRCLE_FRAMES;

    //船这类大体积用更大的圆
    new (&crashBoxCircle) CrashBox_Circle(detailPos,GameValue::getBasicRadius(SORT_ARMY,type));

    update();
}

//重算攻击力、防御力、攻击距离（血量/速度由 Human::update 处理）
void Army::update(){
    Human::update();

    SafePtr<Development> sci = getPlayerScience().lock();

    const std::pair<Double,Double> baseAtk = GameValue::getBasicAttack(sort,type);
    const Double atkRate = sci ? sci->get_rate_Attack(sort,type,armyClass,attackType) : Double(1);
    const I32 atkAdd = sci ? sci->get_addition_Attack(sort,type,armyClass,attackType) : 0;
    attackClose = applyTech(baseAtk.first,atkRate,atkAdd);
    attackShoot = applyTech(baseAtk.second,atkRate,atkAdd);

    const std::pair<Double,Double> baseDef = GameValue::getBasicDefence(sort,type);
    const Double dcRate = sci ? sci->get_rate_Defence(sort,type,armyClass,ATTACKTYPE_CLOSE) : Double(1);
    const I32 dcAdd = sci ? sci->get_addition_Defence(sort,type,armyClass,ATTACKTYPE_CLOSE) : 0;
    defenceClose = applyTech(baseDef.first,dcRate,dcAdd);

    const Double dsRate = sci ? sci->get_rate_Defence(sort,type,armyClass,ATTACKTYPE_SHOOT) : Double(1);
    const I32 dsAdd = sci ? sci->get_addition_Defence(sort,type,armyClass,ATTACKTYPE_SHOOT) : 0;
    defenceShoot = applyTech(baseDef.second,dsRate,dsAdd);

    const std::pair<Double,Double> baseRange = GameValue::getBasicAttackRange(sort,type);
    const I32 rangeAdd = sci ? sci->get_addition_DisAttack(sort,type,armyClass,attackType) : 0;
    const Double baseDistance = (attackType == ATTACKTYPE_SHOOT) ? baseRange.second : baseRange.first;
    minAttackRange = static_cast<I32>(baseDistance) + rangeAdd;
}
