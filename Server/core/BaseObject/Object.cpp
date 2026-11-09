#include "Object.h"
#include "Server/core/Module/Player/Player.h"

U64 Object::globalSNIndex=1;	//给全局对象分配SN

namespace{
//判断这个对象用矩形碰撞盒还是圆形碰撞盒：
//人、动物、投掷物是圆；建筑、资源建筑、静态资源统一是矩形
bool IsRectBox(I32 sort){
    return sort==SORT_BUILDING||sort==SORT_BUILDING_RESOURCE||sort==SORT_STATICRES;
}

//负坐标要向下取整：直接整除会朝 0 截断，负坐标会落到相邻的块
I32 FloorDiv(I32 value,I32 side){
    I32 q=value/side;
    I32 r=value%side;
    if(r<0){
        --q;
    }
    return q;
}
}

Object::Object()
{
    SN=globalSNIndex++;
    //先给个"无主"的默认值：setBelong 遇到负数不会赋值，不初始化就会留下脏数据
    playerID=-1;
    teamID=-1;
    sort=-1;
    type=-1;
    vision=0;
}

I32 Object::getSort(){
    return sort;
}
I32 Object::getType(){
    return type;
}
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
U64 Object::getSN(){
    return SN;
}
void Object::setPos(const IVec2&pos){
    detail_pos=pos;

    const I32 side=BLOCKSIDELENGTH;
    if(side<=0){
        block_pos=IVec2(0,0);
        return;
    }
    block_pos.x=FloorDiv(pos.x,side);
    block_pos.y=FloorDiv(pos.y,side);
}

bool Object::checkCrash(const Object* obj){
    if(obj==nullptr||obj==this){
        return false;
    }

    const bool selfRect=IsRectBox(sort);
    const bool otherRect=IsRectBox(obj->sort);

    if(selfRect&&otherRect){
        return crashBoxRect.IsCrash(obj->crashBoxRect);
    }
    if(!selfRect&&!otherRect){
        return crashBoxCircle.IsCrash(obj->crashBoxCircle);
    }
    if(selfRect){
        return crashBoxRect.IsCrash(obj->crashBoxCircle);
    }
    return obj->crashBoxRect.IsCrash(crashBoxCircle);
}

void Object::tick(){
    //基类不做事，各对象的逐帧行为由派生类/内核负责
}

void Object::setBelong(I32 playerID,I32 teamID){
    if(playerID>=0)this->playerID=playerID;
    if(teamID>=0)this->teamID=teamID;
}
std::pair<I32,I32> Object::getBelong(){
    return std::make_pair(playerID,teamID);
}
