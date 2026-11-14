#include "Object.h"
#include "Server/core/Module/Player/Player.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

#include <new>
#include <utility>

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
    //name 在派生类构造函数里由 ObejctBaseValue 的名称表初始化好，这里直接返回缓存值
    return name;
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

void Object::rebuildCrashBox(){
    const std::pair<I32,I32> box = ObjectBaseValue::getCrashBox(sort,type);
    if(box.first == static_cast<I32>(BOX_RECT)){
        new (&crashBoxRect) CrashBox_Rect(detail_pos,box.second,box.second);
    }else{
        new (&crashBoxCircle) CrashBox_Circle(detail_pos,box.second);
    }
}

bool Object::checkCrash(const Object*obj){
    if(obj == nullptr){
        return false;
    }
    //联合体不记录当前是圆还是矩形，两边都按基础数值表的口径来判断
    const bool thisIsRect =
        ObjectBaseValue::getCrashBox(sort,type).first == static_cast<I32>(BOX_RECT);
    const bool otherIsRect =
        ObjectBaseValue::getCrashBox(obj->sort,obj->type).first == static_cast<I32>(BOX_RECT);
    if(thisIsRect){
        return otherIsRect ? crashBoxRect.IsCrash(obj->crashBoxRect)
                           : crashBoxRect.IsCrash(obj->crashBoxCircle);
    }
    return otherIsRect ? crashBoxCircle.IsCrash(obj->crashBoxRect)
                       : crashBoxCircle.IsCrash(obj->crashBoxCircle);
}
void Object::tick(){
    //待完成
}
void Object::refreshByScience(){
    //默认实现：公有资源、投掷物等没有跟随科技的缓存值，重算时什么都不用做。
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