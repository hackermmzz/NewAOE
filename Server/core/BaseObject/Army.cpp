#include "Army.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/config/ObjectBaseValue.h"

Army::Army(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience): Human(SORT_ARMY, detailPos, playerScience)
{
    this->type = type;              //具体兵种，保持 AT_* 不变，供基础数值表检索
    this->armyClass = ARMY_INFANTRY;  //兵种大类，供科技按大类给加成
    switch(type){
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
            this->armyClass =ARMY_INFANTRY;
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
            this->armyClass =ARMY_ARCHER;
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
            this->armyClass =ARMY_RIDER;
            break;
        }
        case AT_PRIEST:
        {
            this->armyClass =ARMY_FLAMEN;
            break;
        }
        case AT_STONE_THROWER:
        case AT_CATAPULT:
        case AT_HEAVY_CATAPULT:
        case AT_BALLISTA:
        case AT_HELEPOLIS:
        {
            this->armyClass =ARMY_SIEGE;
            break;
        }
        case AT_SCOUT_SHIP:
        case AT_WAR_GALLEY:
        case AT_TRIREME:
        case AT_CATAPULT_TRIREME:
        case AT_FIRESAILBOAT:
        {
            this->armyClass =ARMY_SHIP;
            break;
        }
        default:break;

    }
    //基础视野与名称
    vision = ObjectBaseValue::getBasicVision(SORT_ARMY,type);
    name = ObjectBaseValue::getObjectName(SORT_ARMY,type);
    //构造函数里血量/攻防/速度还是空的，先按当前科技填一遍
    Human::refresh();
    //碰撞盒按基础数值表建立
    rebuildCrashBox();
}

I32 Army::getArmyClass() const{return armyClass;}