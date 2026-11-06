#include "Human.h"

Human::Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience, I32 playerRepresent){
    this->sort = sort;
    this->detail_pos = detailPos;
    this->playerScience = playerScience;
    SN = playerRepresent;
    transported = false;
}

bool Human::getTransported() const{
    return transported;
}

void Human::setTransported(bool value){
    transported = value;
}

