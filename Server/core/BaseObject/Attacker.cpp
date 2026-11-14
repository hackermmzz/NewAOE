#include "Attacker.h"
#include "Server/core/Public/var/Army.h"

Double Attacker::getAttackBasic()
{
    return attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
}

Double Attacker::getAttackAdditon()
{
    //等待Development完成
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