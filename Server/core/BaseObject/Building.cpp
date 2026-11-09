#include "Building.h"

#include <new>
#include <utility>

#include "Server/core/Public/var/Army.h"

Building::Building(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent):Object()
{
    sort = SORT_BUILDING;
    this->type = type;
    this->playerScience = playerScience;
    this->Percent = percent;
    //Percent 已经到 100 的就是建好的（比如地图预放置的建筑），否则要靠建造进度推进
    this->constructed = (Percent >= 100);

    //先把血量和攻防清 0，update() 要靠"旧上限 - 旧血量"算已损失的伤害
    maxBlood = Double::Zero();
    currentBlood = Double::Zero();
    defenceClose = Double::Zero();
    defenceShoot = Double::Zero();
    attackClose = Double::Zero();
    attackShoot = Double::Zero();
    attackType = ATTACKTYPE_SHOOT;
    range = false;
    rangeEffect = 0;
    minAttackRange = 0;
    attackCircle = 0;
    attackCurrentTick = 0;
    attackInteval = 0;

    fundation = GameValue::getBasicFoundation(sort,type);
    vision = GameValue::getBasicVision(sort,type);
    attackInteval = GameValue::getAttackInterval(sort,type);
    attackCircle = ATTACK_CIRCLE_FRAMES;

    //blockPos 是块坐标，detail_pos 取建筑的左上角
    const I32 side = BLOCKSIDELENGTH;
    setPos(IVec2(blockPos.x*side,blockPos.y*side));

    //矩形碰撞盒：边长由地基决定，SMALL 1 块、MIDDLE 2 块、BIG 3 块
    I32 blocks = 3;
    if(fundation == FOUNDATION_SMALL){
        blocks = 1;
    }else if(fundation == FOUNDATION_MIDDLE){
        blocks = 2;
    }
    new (&crashBoxRect) CrashBox_Rect(detail_pos,blocks*side,blocks*side);

    update();
}

//重算血量、攻防、射程（建筑没有移动速度）
void Building::update(){
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

    //建筑只有箭塔有攻击，其余行的攻击力是 0，套科技也是 0
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

    const std::pair<Double,Double> baseRange = GameValue::getBasicAttackRange(sort,type);
    const I32 rangeAdd = sci ? sci->get_addition_DisAttack(sort,type,ARMY_INFANTRY,attackType) : 0;
    minAttackRange = static_cast<I32>(baseRange.second) + rangeAdd;
}

bool Building::isConstructed(){
    if(Percent == 100){
        constructed = true;
    }
    return constructed;
}

//建造进度由内核/指令模块推进：每帧把 Percent 加上去，到 100 就算建好
void Building::setPercent(I32 percent){
    if(percent < 0){
        percent = 0;
    }
    if(percent > 100){
        percent = 100;
    }
    Percent = percent;
    if(Percent >= 100){
        constructed = true;
    }
}

I32 Building::getPercent(){
    return Percent;
}

bool Building::isMatchResourceType(int productSort){
    if(type == BUILDING_CENTER){
        return true;
    }
    if(type == BUILDING_GRANARY && productSort == PRODUCTSORT_GRANARYFOOD){
        return true;
    }
    if(type == BUILDING_STOCK && (productSort == PRODUCTSORT_STOCKFOOD || productSort == PRODUCTSORT_WOOD
                                  || productSort == PRODUCTSORT_STONE || productSort == PRODUCTSORT_GOLD)){
        return true;
    }
    return false;
}

/* ---------------- 供 UI 显示：基础值与加成值分开 ----------------
   基础值一律现查表（不含科技），加成值 = 缓存的最终值 - 基础值。 */

Double Building::getAttackBasic(){
    const std::pair<Double,Double> base = GameValue::getBasicAttack(sort,type);
    return (attackType == ATTACKTYPE_SHOOT) ? base.second : base.first;
}

Double Building::getAttackAdditon(){
    const Double actual = (attackType == ATTACKTYPE_SHOOT) ? attackShoot : attackClose;
    return actual - getAttackBasic();
}

Double Building::getDefenceCloseBasic(){
    return GameValue::getBasicDefence(sort,type).first;
}

Double Building::getDefenceShootBasic(){
    return GameValue::getBasicDefence(sort,type).second;
}

Double Building::getDefenceCloseAddition(){
    return defenceClose - getDefenceCloseBasic();
}

Double Building::getDefenceShootAddition(){
    return defenceShoot - getDefenceShootBasic();
}
