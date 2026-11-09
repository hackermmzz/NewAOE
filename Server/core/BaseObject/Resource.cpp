#include "Resource.h"

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
    cntLeft -= decayRate;
    if(cntLeft < Double::Zero()){
        cntLeft = Double::Zero();
    }
}
bool Resource::isSurplus(){
    return cntLeft > Double::Zero();
}