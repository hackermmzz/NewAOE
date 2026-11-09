#include "Attacker.h"
#include "Server/core/Public/var/Army.h"

Double Attacker::getAttackBasic()
{
    return attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
}

//默认实现：没有科技的对象（动物）加成恒为 0；
//Human/Building 会覆盖成"实际值 - 基础值"，供 UI 显示
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
