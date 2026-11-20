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
    reduceResource(decayRate);
}
void Resource::reduceResource(Double ResourceAmount){
    if(ResourceAmount <= Double::Zero()){return;}
    cntLeft -=ResourceAmount;
    if(cntLeft < Double::Zero()){
        cntLeft = Double::Zero();
        gatherAble=false;
    }
}
bool Resource::isSurplus(){
    return cntLeft > Double::Zero();
}