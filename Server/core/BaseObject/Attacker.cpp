#include "Attacker.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/config/Core.h"

Attacker::Attacker()
    : attackClose(Double::Zero())
    , attackShoot(Double::Zero())
    , attackType(ATTACKTYPE_CLOSE)
    , range(false)
    , rangeEffect(0)
    , minAttackRange(0)
    , attackCircle(ATTACK_CIRCLE_FRAMES)
    , attackCurrentTick(0)
    , attackInteval(0)
{
}

//默认实现什么都不做，需要响应命中的派生类自行覆写
void Attacker::notifyHit(){
}

Double Attacker::getAttackBasic()
{
    return attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
}

Double Attacker::getAttackAdditon()
{
    //科技的加成已经在 refreshByScience() 里并进 attackClose / attackShoot 了，
    //所以 getAttackBasic() 取到的就是最终攻击力，这里不再叠加，返回 0。
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