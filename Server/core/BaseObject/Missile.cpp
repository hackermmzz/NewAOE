#include "Missile.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/config/ObjectBaseValue.h"

Missile::Missile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack)
{
    sort=SORT_MISSILE;
    this->type=type;
    //归属由 Player::newMissile 通过 setBelong 写入，这里先给个中立值
    playerID = -1;
    teamID = -1;
    this->attacker=attacker;
    this->attackee=attackee;
    this->playerScience=playerScience;

    attackType = ATTACKTYPE_SHOOT;
    attackClose = Double::Zero();
    attackShoot = Double(attack);
    //巨石这类投掷物带范围伤害
    range = (type == Missile_Boulders);
    rangeEffect = range ? 1 : 0;
    minAttackRange = 0;
    attackInteval = 0;
    attackCurrentTick = 0;
    speed = ObjectBaseValue::getBasicSpeed(SORT_MISSILE,type);
    vision = 0;

    //攻击者/目标可能在这一帧里已经被删掉了，取不到位置就退回起点，不能直接解引用
    SafePtr<Object> from = attacker.lock();
    SafePtr<Object> to = attackee.lock();
    const IVec2 startPos = (from != nullptr) ? from->getDetailPos() : IVec2();
    setPos(startPos);
    targetDetailPos = (to != nullptr) ? to->getDetailPos() : startPos;

    //设置名称
    name = ObjectBaseValue::getObjectName(SORT_MISSILE,type);

    rebuildCrashBox();
}
