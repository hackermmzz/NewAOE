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
void Resource::decayResource(Double decayAmount){
    //按调用方给的数量扣（采集、被破坏这类），同样不把剩余量扣成负数。
    //非正数直接忽略：传 0 什么也不做，传负数没有意义。
    if(decayAmount <= Double::Zero()){
        return;
    }
    cntLeft -= decayAmount;
    if(cntLeft < Double::Zero()){
        cntLeft = Double::Zero();
    }
}
bool Resource::isSurplus(){
    return cntLeft > Double::Zero();
}