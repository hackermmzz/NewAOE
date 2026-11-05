#include "MoveObject.h"

void MoveObject::setPath(const stack<IVec2> &path, const IVec2 &targetDetailPoint)
{
    this->path = path;
    this->targetDetailPos = targetDetailPoint;
}

FORCE_INLINE IVec2 MoveObject::getTargetDetailPos()
{
    return targetDetailPos;
}

FORCE_INLINE IVec2 MoveObject::nextPos(){
    //待完成
}

Double getSpeed(){
    //待完成
}