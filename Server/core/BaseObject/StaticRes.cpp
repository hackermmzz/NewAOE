#include "StaticResource.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/StaticResource.h"

#include <new>

namespace{
//各资源产出的资源种类
I32 StaticProductSort(I32 type){
    switch(type){
    case STATICRES_Bush:
        return PRODUCTSORT_GRANARYFOOD;
    case STATICRES_Stone:
        return PRODUCTSORT_STONE;
    case STATICRES_Gold:
        return PRODUCTSORT_GOLD;
    case STATICRES_Fish:
        return PRODUCTSORT_STOCKFOOD;
    default:
        return PRODUCTSORT_WOOD;
    }
}
}

StaticRes::StaticRes(I32 type,IVec2 detailPos)
{
    sort = SORT_STATICRES;
    this->type = type;
    //浆果/石/金/鱼都是中立资源，playerID/teamID 保持 Object 构造里的 -1

    vision = 0;
    setPos(detailPos);

    maxCnt = GameValue::getResourceBasicMaxCnt(SORT_STATICRES,type);
    cntLeft = maxCnt;
    decayRate = GameValue::getResourceBasicDecayRate(SORT_STATICRES,type);
    resourceSort = StaticProductSort(type);
    gatherAble = true;

    //静态资源统一用矩形盒，按 1 块见方
    const I32 side = BLOCKSIDELENGTH;
    new (&crashBoxRect) CrashBox_Rect(detailPos,side,side);
}
