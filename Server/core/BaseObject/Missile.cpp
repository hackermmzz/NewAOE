#include "Missile.h"
#include "Server/core/Public/var/Army.h"

Missile::Missile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack)
{
    attackType = ATTACKTYPE_SHOOT;
    attackClose = 0;
    attackShoot = attack;
    range = false;
    if(type == Missile_Boulders){
        range = true;
    }
    //尚未完成
    /*this->type = type;
    this->attacker = attacker;
    this->attackee = attackee;
    this->playerScience = playerScience;
    this->attackType = attack;*/
}