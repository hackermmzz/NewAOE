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
void Building::refreshByScience(){
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){
        return;
    }

    const I32 objectSort = getSort();
    const I32 objectType = getType();

    //---- 血量：建筑学等科技会改这一项 ----
    maxBlood = ObjectBaseValue::getBasicBlood(objectSort,objectType)
             * science->get_rate_Blood(objectSort,objectType)
             + Double(science->get_addition_Blood(objectSort,objectType));
    if(currentBlood > maxBlood){
        currentBlood = maxBlood;
    }

    //攻击/护甲/射程暂不在这里重算：BuildingBase 目前只收了口径明确的视野与占地，
    //塔的攻击数值还没有进表，直接重算会把塔的攻击清成 0。
    //等建筑的攻击列补进 ObejctBaseValue 之后再一并处理。
}
