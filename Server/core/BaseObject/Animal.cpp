#include "Animal.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Animal.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

#include <new>

namespace{
//动物尸体能采出来的东西：树是木头，其余是猎物（走仓库食物）
I32 AnimalProductSort(I32 type){
    if(type == ANIMAL_TREE){
        return PRODUCTSORT_WOOD;
    }
    return PRODUCTSORT_STOCKFOOD;
}

//只有狮子、鳄鱼这类会主动攻击的算敌对，其余是中立

I32 AnimalFriendly(I32 type){
    if(type == ANIMAL_LION || type == ANIMAL_ALLIGATOR || type == ANIMAL_ALLIGATOR_KING){
        return FRIENDLY_ENEMY;
    }
    return FRIENDLY_NULL;
}
}

Animal::Animal(I32 type,IVec2 detailPos)
{
    sort = SORT_ANIMAL;
    this->type = type;
    //动物是中立单位，playerID/teamID 保持 Object 构造里的 -1

    setPos(detailPos);

    vision = GameValue::getBasicVision(SORT_ANIMAL,type);
    friendly = AnimalFriendly(type);

    maxBlood = GameValue::getBasicBlood(SORT_ANIMAL,type);
    currentBlood = maxBlood;
    defenceClose = Double::Zero();
    defenceShoot = Double::Zero();

    attackType = ATTACKTYPE_CLOSE;
    attackClose = GameValue::getBasicAttack(SORT_ANIMAL,type).first;
    attackShoot = Double::Zero();
    range = false;
    rangeEffect = 0;
    //有额外够到距离的（鳄鱼 2 格）用表里的值，其余贴身
    const Double animalRange = GameValue::getBasicAttackRange(SORT_ANIMAL,type).second;
    minAttackRange = (animalRange > Double::Zero()) ? static_cast<I32>(animalRange) : DISTANCE_ATTACK_CLOSE;
    attackCircle = ATTACK_CIRCLE_FRAMES;
    attackCurrentTick = 0;
    attackInteval = GameValue::getAttackInterval(SORT_ANIMAL,type);
    speed = GameValue::getBasicSpeed(SORT_ANIMAL,type);

    //动物死了才能采，所以刚创建时不可采集
    maxCnt = GameValue::getResourceBasicMaxCnt(SORT_ANIMAL,type);
    cntLeft = maxCnt;
    decayRate = Double::Zero();
    resourceSort = AnimalProductSort(type);
    gatherAble = false;

    new (&crashBoxCircle) CrashBox_Circle(detailPos,GameValue::getBasicRadius(SORT_ANIMAL,type));
}

I32 Animal::getFriendly(){
    return friendly;
}

//被打死之后尸体才可以采
void Animal::notifyAttacked(){
    if(isDie()){
        gatherAble = true;
    }
}
