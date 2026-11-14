#include "Object.h"
#include "Server/core/Module/Player/Player.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

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
/*I32 Object::getPlayerRepresent(){
    return SN;
}
void Object::setPlayerRepresent(I32 playerRepresent){
    SN = playerRepresent;
}*/
IVec2 Object::getBlockPos(){
    return block_pos;
}
IVec2 Object::getDetailPos(){
    return detail_pos;
}
string Object::getName(){
    return GameValue::getObjectName(playerID,teamID,sort,type,SN);
}
I32 Object::getVision(){
    return vision;
}
I32 Object::getglobalNum(){
    return SN;
}
void Object::setPos(const IVec2&pos){
    detail_pos = pos;
    block_pos.x=detail_pos.x/BLOCKSIDELENGTH;
    block_pos.y=detail_pos.y/BLOCKSIDELENGTH;
}

bool Object::checkCrash(const Object*obj){
    //待完成
}
void Object::tick(){
    //待完成
}
/*void Object::setplayer(WeakPtr<Player> player){
    this->player = player;
}
void Object::getplayer(WeakPtr<Player> player){
    player = this->player;
}*/
void Object::setBelong(I32 playerID,I32 teamID){
    if(playerID>=0)this->playerID = playerID;
    if(teamID>=0)this->teamID = teamID;
}
std::pair<I32,I32> Object::getBelong(){
    return std::make_pair(playerID,teamID);
}