#include "Missile.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/Missile.h"

#include <new>

Missile::Missile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack)
{
    sort = SORT_MISSILE;
    this->type = type;
    this->attacker = attacker;
    this->attackee = attackee;
    this->playerScience = playerScience;

    vision = 0;
    speed = milliToDouble(MISSILE_SPEED_MILLI);

    //投掷物的伤害在创建时就定死（取自攻击者当时的攻击力），之后不再吃科技
    attackType = ATTACKTYPE_SHOOT;
    attackClose = Double::Zero();
    attackShoot = Double(attack);
    range = (type == Missile_Boulders);     //投石车砸出的巨石是范围伤害
    rangeEffect = 0;
    minAttackRange = 0;
    attackCircle = 0;
    attackCurrentTick = 0;
    attackInteval = 0;

    //起点跟着攻击者，终点跟着目标；任何一方已经失效都退回原点，避免解引用空指针
    SafePtr<Object> from = attacker.lock();
    SafePtr<Object> to = attackee.lock();
    setPos(from ? from->getDetailPos() : IVec2(0,0));
    targetDetailPos = to ? to->getDetailPos() : detail_pos;

    //投掷物不参与碰撞，半径给 0
    new (&crashBoxCircle) CrashBox_Circle(detail_pos,0);
}
