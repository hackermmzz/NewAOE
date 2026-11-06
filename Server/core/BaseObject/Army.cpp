#include "Army.h"


Army::Army(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience , I32 playerRepresent)
    : Human(SORT_ARMY, detailPos, playerScience, playerRepresent)
{
    this->armyClass = type;
    this->playerScience = playerScience;
    //尚未完成

}