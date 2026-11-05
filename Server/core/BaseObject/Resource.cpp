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
void Resource::decayResource(){	//腐烂资源
    resourceSort -= decayRate;
}
bool Resource::isSurplus(){	//资源是否还有剩余
    return cntLeft > Double::Zero();
}