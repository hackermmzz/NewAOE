#include "Attacker.h"
#include "Server/core/Public/var/Army.h"

Double Attacker::getAttackBasic()
{
    return attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
}

//攻击的科技加成由 update() 直接烘进 attackClose/attackShoot，
//这里默认返回 0，避免和缓存的最终值重复相加
Double Attacker::getAttackAdditon()
{
    return Double::Zero();
}

Double Attacker::getAttack()
{
    return getAttackBasic() + getAttackAdditon();
}

I32 Attacker::getAttackCircle()
{
    return attackCircle;
}

I32 Attacker::getCurrentAttackTick()
{
    return attackCurrentTick;
}

I32 Attacker::getAttackInteval()
{
    return attackInteval;
}

void Attacker::resetAttackTick()
{
    attackCurrentTick = 0;
}

//命中时由内核调用：派生类要加特效/吸血/溅射就在这里覆盖
void Attacker::notifyHit()
{
}
