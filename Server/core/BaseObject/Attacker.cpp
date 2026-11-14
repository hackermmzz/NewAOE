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
    //默认实现：没有科技加成的对象（动物、投掷物）加成恒为 0。
    //Human / Building 覆写成「当前攻击力 - ObejctBaseValue 里的基础值」。
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