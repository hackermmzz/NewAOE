#include "StaticResource.h"
#include "Server/core/Public/config/ObejctBaseValue.h"
#include "Server/core/Public/var/Sort.h"

namespace{
//静态资源能采出来的产物种类（上交目标由指令/内核按这个判断）
I32 StaticResourceProductSort(I32 type){
    switch(type){
    case STATICRES_Bush:
    case STATICRES_Fish:
        return PRODUCTSORT_GRANARYFOOD;   //可以上交谷仓或市镇中心
    case STATICRES_Stone:
        return PRODUCTSORT_STONE;
    case STATICRES_Gold:
        return PRODUCTSORT_GOLD;
    default:
        return PRODUCTSORT_GRANARYFOOD;
    }
}
}

StaticRes::StaticRes(I32 type,IVec2 detailPos)
{
    sort = SORT_STATICRES;
    this->type = type;
    //静态资源不属于任何玩家
    playerID = -1;
    teamID = -1;

    setPos(detailPos);

    vision = ObjectBaseValue::getBasicVision(SORT_STATICRES,type);
    name = ObjectBaseValue::getObjectName(SORT_STATICRES,type);
    //资源相关的数值
    maxCnt = ObjectBaseValue::getResourceBasicMaxCnt(SORT_STATICRES,type);
    cntLeft = maxCnt;
    decayRate = ObjectBaseValue::getResourceDecayRate(SORT_STATICRES,type);
    resourceSort = StaticResourceProductSort(type);
    gatherAble = true;

    rebuildCrashBox();
}
