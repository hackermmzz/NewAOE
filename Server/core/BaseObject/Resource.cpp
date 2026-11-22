#include "Resource.h"

Resource::Resource()
    : cntLeft(Double::Zero())
    , maxCnt(Double::Zero())
    , decayRate(Double::Zero())
    , resourceSort(-1)
    , gatherAble(false)
{
}

bool Resource::isGatherAble(){
    return gatherAble;
}
I32 Resource::getResourceSort(){
    return resourceSort;
}
Double Resource::getCntLeft(){
    return cntLeft;
}
Double Resource::getMaxCnt(){
    return maxCnt;
}
void Resource::decayResource(){
    if(decayRate <= Double::Zero()){
        return;
    }
    cntLeft -= decayRate;
    //腐烂不能把剩余量扣成负数
    if(cntLeft < Double::Zero()){
        cntLeft = Double::Zero();
    }
}
bool Resource::isSurplus(){
    return cntLeft > Double::Zero();
}