#include "Building.h"



Building::Building(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent):Object()
{
    sort = SORT_BUILDING;
    this->type = type;
    if(type ==BUILDING_HOME || type == BUILDING_ARROWTOWER){
        this->fundation = FOUNDATION_MIDDLE;
    }else if(type == BUILDING_WALL){
        this->fundation = FOUNDATION_SMALL;
    }else{
        this->fundation = FOUNDATION_BIG;
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