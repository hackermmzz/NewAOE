#include "Building.h"
#include "Server/core/Public/config/ObjectBaseValue.h"



Building::Building(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent):Object(){
    sort = SORT_BUILDING;
    this->type = type;
    switch(type){
    case BUILDING_HOME:
    case BUILDING_WATCH_TOWER:
    case BUILDING_SENTRY_TOWER:
    case BUILDING_GUARD_TOWER:
    case BUILDING_BALLISTA_TOWER:
        this->fundation = FOUNDATION_MIDDLE;
        break;
    case BUILDING_SMALL_WALL:
    case BUILDING_MEDIUM_WALL:
    case BUILDING_FORTIFIED_WALL:
        this->fundation = FOUNDATION_SMALL;
        break;
    case BUILDING_WONDER:
        this->fundation = FOUNDATION_HUGE;
        break;
    default:
        this->fundation = FOUNDATION_BIG;
        break;
    }
    this->block_pos = blockPos;
    //建筑给的是区块坐标，细节坐标按区块边长换算出来
    this->detail_pos = IVec2(static_cast<I32>(BLOCKSIDELENGTH * Double(blockPos.x)),static_cast<I32>(BLOCKSIDELENGTH * Double(blockPos.y)));
    vision = ObjectBaseValue::getBasicVision(sort,type);
    this->playerScience = playerScience;
    this->constructed = false;
    if(percent>=100)this->constructed = true;
    //设置名称
    name = ObjectBaseValue::getObjectName(sort,type);
    this->Percent=percent;
    // 内部血量统一按「满血」存放，建造进度只在读取时折算（见 getMaxBlood / getBlood），
    maxBlood = computeFullBlood();
    currentBlood = maxBlood;
    refreshDefence();
    //碰撞盒按基础数值表建立
    rebuildCrashBox();
}
bool Building::isConstructed(){
    if(Percent>=100)constructed=true;
    return constructed;
}
bool Building::isMatchResourceType(int productSort){
    if(type == BUILDING_CENTER){
        return true;
    }
    if(type == BUILDING_GRANARY&&productSort == PRODUCTSORT_GRANARYFOOD){
        return true;
    }
    if(type == BUILDING_STOCK&&(productSort == PRODUCTSORT_STOCKFOOD||productSort == PRODUCTSORT_WOOD||productSort == PRODUCTSORT_STONE||productSort == PRODUCTSORT_GOLD)){
        return true;
    }
    return false;
}
Double Building::computeFullBlood(){
    const I32 objectSort = getSort();
    const I32 objectType = getType();

    //基础血量 × 科技血量加成 + 科技额外加成
    Double result = ObjectBaseValue::getBasicBlood(objectSort,objectType);
    SafePtr<Development> science = playerScience.lock();
    if(science != nullptr){
        result = result * science->get_rate_Blood(objectSort,objectType)
               + Double(science->get_addition_Blood(objectSort,objectType));
    }

    //满血值不允许为 0
    if(result <= Double::Zero()){
        result = Double(1);
    }
    return result;
}

void Building::build(I32 percent){
    //建造进度是累加的：每次传入本次推进的百分比
    Percent += percent;
    if(Percent > 100){
        Percent = 100;
        constructed=true;
    }
    if(Percent < 0){
        Percent = 0;
    }
}

Double Building::getMaxBlood(){
    //建成后走正常逻辑
    if(isConstructed())return maxBlood;
    //施工中：满血值 × 建造进度
    Double scaled = maxBlood * Double(Percent) / Double(100);
    //进度为 0 时算出来是 0，保底给 1 点，避免建筑一落地就是死的
    return scaled <= Double::Zero() ? Double(1) : scaled;
}

Double Building::getBlood(){
    //建成后走正常逻辑
    if(isConstructed())return currentBlood;
    //施工中：当前血量同样按进度折算；已经被打掉的建筑不再保底
    Double result=getMaxBlood()-(maxBlood-currentBlood);
    if(result<= Double::Zero())return Double::Zero();
    return result;
}

//---- UI 用的「基础值 / 加成值」拆分 ----
//基础值一律取 ObejctBaseValue，加成值 = 当前值（已含科技）- 基础值。

Double Building::getAttackBasic(){
    const std::pair<Double,Double> base =ObjectBaseValue::getBasicAttack(getSort(),getType());
    return attackType == ATTACKTYPE_SHOOT ? base.second : base.first;
}

Double Building::getAttackAdditon(){
    const Double current = attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
    return current - getAttackBasic();
}

Double Building::getDefenceCloseBasic(){
    return ObjectBaseValue::getBasicDefence(getSort(),getType()).first;
}

Double Building::getDefenceShootBasic(){
    return ObjectBaseValue::getBasicDefence(getSort(),getType()).second;
}

Double Building::getDefenceCloseAddition(){
    return defenceClose - getDefenceCloseBasic();
}

Double Building::getDefenceShootAddition(){
    return defenceShoot - getDefenceShootBasic();
}
void Building::refresh(){
    refreshBlood();
    refreshDefence();
    refreshAttack();
}
void Building::refreshBlood(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr||isDie()){return;}
    Double newmaxBlood=ObjectBaseValue::getBasicBlood(sort,type)*science->get_rate_Blood(sort,type)+science->get_addition_Blood(sort,type);
    currentBlood+=newmaxBlood-maxBlood;
    maxBlood=newmaxBlood;
}
void Building::refreshDefence(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr||isDie()){return;}
    defenceClose=ObjectBaseValue::getBasicDefence(getSort(),getType()).first*science->get_rate_Defence(sort,type,-1,ATTACKTYPE_CLOSE)+science->get_addition_Defence(sort,type,-1,ATTACKTYPE_CLOSE);
    defenceShoot=ObjectBaseValue::getBasicDefence(getSort(),getType()).second*science->get_rate_Defence(sort,type,-1,ATTACKTYPE_SHOOT)+science->get_addition_Defence(sort,type,-1,ATTACKTYPE_SHOOT);

}
void Building::refreshAttack(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr||isDie()){return;}
    const std::pair<Double,Double> attackBase =ObjectBaseValue::getBasicAttack(sort,type);
    I32 armyClass=-1;
    if(attackBase.first > Double::Zero()){
        attackClose = attackBase.first*science->get_rate_Attack(sort,type,armyClass,ATTACKTYPE_CLOSE)+ Double(science->get_addition_Attack(sort,type,armyClass,ATTACKTYPE_CLOSE));
    }else{
        attackClose = Double::Zero();
    }
    if(attackBase.second > Double::Zero()){
        attackShoot = attackBase.second * science->get_rate_Attack(sort,type,armyClass,ATTACKTYPE_SHOOT) + Double(science->get_addition_Attack(sort,type,armyClass,ATTACKTYPE_SHOOT));
    }else{
        attackShoot = Double::Zero();
    }
    const std::pair<I32,I32> rangeBase = ObjectBaseValue::getBasicAttackRange(sort,type);
    attackType = attackBase.second > Double::Zero() ? ATTACKTYPE_SHOOT : ATTACKTYPE_CLOSE;
    rangeEffect = rangeBase.first;
    range = rangeBase.first > 0;
    minAttackRange = rangeBase.second+science->get_addition_DisAttack(sort,type,armyClass,attackType);
    attackCircle=playerScience.lock()->get_speed_attack(sort,type,armyClass)[1];
    attackInteval=playerScience.lock()->get_speed_attack(sort,type,armyClass)[0];
}

