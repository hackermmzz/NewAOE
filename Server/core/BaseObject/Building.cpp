#include "Building.h"
#include "Server/core/Public/config/ObejctBaseValue.h"



Building::Building(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent):Object()
{
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
    default:
        this->fundation = FOUNDATION_BIG;
        break;
    }
    this->block_pos = blockPos;
    this->playerScience = playerScience;
    this->constructed = false;
    //设置名称
    string tmp="Building---Type:";
    tmp+=std::to_string(type);
    tmp+="---SN:";
    name=tmp+std::to_string(SN);
    this->Percent=Percent;
    // 内部血量统一按「满血」存放，建造进度只在读取时折算（见 getMaxBlood / getBlood），
    // 这样科技变化时只需要刷新一次满血值，不必关心当前盖到几成。
    maxBlood = computeFullBlood();
    currentBlood = maxBlood;
}
bool Building::isConstructed(){
    if(Percent==100)constructed=true;
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
    if(percent > 100){
        percent = 100;
    }
    if(percent < 0){
        percent = 0;
    }
    this->Percent = percent;
}

Double Building::getMaxBlood(){
    //建成后走正常逻辑
    if(Percent >= 100){
        return maxBlood;
    }
    //施工中：满血值 × 建造进度
    Double scaled = maxBlood * Double(Percent) / Double(100);
    //进度为 0 时算出来是 0，保底给 1 点，避免建筑一落地就是死的
    return scaled <= Double::Zero() ? Double(1) : scaled;
}

Double Building::getBlood(){
    //建成后走正常逻辑
    if(Percent >= 100){
        return currentBlood;
    }
    //施工中：当前血量同样按进度折算；已经被打掉的建筑不再保底
    if(currentBlood <= Double::Zero()){
        return Double::Zero();
    }
    Double scaled = currentBlood * Double(Percent) / Double(100);
    return scaled <= Double::Zero() ? Double(1) : scaled;
}

void Building::refreshByScience(){
    maxBlood = computeFullBlood();
    if(currentBlood > maxBlood){
        currentBlood = maxBlood;
    }

    //攻击/护甲/射程暂不在这里重算：BuildingBase 目前只收了口径明确的视野与占地，
    //塔的攻击数值还没有进表，直接重算会把塔的攻击清成 0。
    //等建筑的攻击列补进 ObejctBaseValue 之后再一并处理。
}
