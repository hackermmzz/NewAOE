#include "MoveObject.h"

void MoveObject::setPath(const stack<IVec2> &path, const IVec2 &targetDetailPoint)
{
    this->path = path;
    this->targetDetailPos = targetDetailPoint;
    //路径的第一个点就是自己脚下时要去掉，否则会原地多停一帧；
    //空栈时不能调 top()，否则直接崩。
    if(!this->path.empty() && this->path.top() == detail_pos){
        this->path.pop();
    }
}

Double MoveObject::getSpeed(){
    return speed;
}
