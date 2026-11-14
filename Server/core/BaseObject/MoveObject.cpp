#include "MoveObject.h"

void MoveObject::setPath(const stack<IVec2> &path, const IVec2 &targetDetailPoint)
{
    this->path = path;
    this->targetDetailPos = targetDetailPoint;
    if(this->path.top() == detail_pos){
        this->path.pop();
    }
}

FORCE_INLINE IVec2 MoveObject::getTargetDetailPos()
{
    return targetDetailPos;
}

FORCE_INLINE IVec2 MoveObject::nextPos(){
    if(!path.empty()){
        IVec2 next = path.top();
        path.pop();
        return next;
    }
    else{
        return targetDetailPos;
    }
}

Double MoveObject::getSpeed(){
    return speed;
}