#include "MoveObject.h"

void MoveObject::setPath(const stack<IVec2> &path, const IVec2 &targetDetailPoint)
{
    this->path = path;
    this->targetDetailPos = targetDetailPoint;
    if(!this->path.empty() && this->path.top() == detail_pos){
        this->path.pop();
    }
}
Double MoveObject::getSpeed(){return speed;}
void MoveObject::refreshSpeed(){return;}