#include "Missile.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Sort.h"

Missile::Missile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack)
{
    sort=SORT_MISSILE;
    this->type=type;
    this->attacker=attacker;
    this->attackee=attackee;
    this->playerScience=playerScience;
    attackType = ATTACKTYPE_SHOOT;
    attackClose = 0;
    attackShoot = attack;
    range = false;
    if(type == Missile_Boulders){
        range = true;
    }
    setPos(attacker.lock()->getDetailPos());
    targetDetailPos=attackee.lock()->getDetailPos();
    vision=0;
    //设置名称
    string tmp="Missile---Type:";
    tmp+=std::to_string(type);
    tmp+="---SN:";
    name=tmp+std::to_string(SN);
    //尚未完成
    /*this->type = type;
    this->attacker = attacker;
    this->attackee = attackee;
    this->playerScience = playerScience;
    this->attackType = attack;*/
}