#include "Object.h"

U64 Object::globalSNIndex=1;	//给全局对象分配SN

Object::Object()
{
    SN=globalSNIndex++;
}

I32 Object::getSort(){
    return sort;
}
I32 Object::getType(){
    return type;
}
I32 Object::getPlayerRepresent(){
    return SN;
}
void Object::setPlayerRepresent(I32 playerRepresent){
    SN = playerRepresent;
}
IVec2 Object::getBlockPos(){
    return block_pos;
}
IVec2 Object::getDetailPos(){
    //return detail_pos;
}
string Object::getName(){
    return name;
}
I32 Object::getVision(){
    return vision;
}
I32 Object::getglobalNum(){
    return globalSNIndex;
}
void Object::setPos(const IVec2&pos){
    detail_pos = pos;
}

bool Object::checkCrash(const Object*obj){
    //待完成
}
void Object::tick(){
    //待完成
}