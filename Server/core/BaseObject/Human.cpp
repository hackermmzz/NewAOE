#include "Human.h"

Human::Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience){
    this->sort = sort;
    setPos(detailPos);
    this->playerScience = playerScience;
    transported = false;
}

bool Human::getTransported() const{
    return transported;
}

void Human::setTransported(bool value){
    transported = value;
}

