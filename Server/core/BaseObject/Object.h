#pragma once

#include <string>
#include "Public/vec/vec.hpp"
#include "Server/core/BaseObject/CrashBox.h"
using namespace std;

class Object
{
protected:
	I32 player;	//所属玩家的编号（如果属于公立资源，那么为-1）
	I32 sort;	//对象的大类型，比如农民，军队等
	I32 type;	//对象具体类型(比如运输船、农民、投石兵等这些具体角色）
	U64 SN;	//全局对象编号
	string name;	//对象名称
	IVec2 detail_pos;	//细节坐标
	IVec2 block_pos;	//区块坐标
	I32 vision;	//视野范围
	union{
		CrashBox_Rect crashBoxRect;	//矩形碰撞包围盒
		CrashBox_Circle crashBoxCircle; //圆形碰撞包围盒
	};
	static U64 globalSNIndex;	//给全局对象分配SN
public:
	Object();
	I32 getSort();
	I32 getType();
    	I32 getPlayerRepresent();
	void setPlayerRepresent(I32 playerRepresent);
	IVec2 getBlockPos();
	Vec2 getDetailPos();
	string getName();
	I32 getVision();
  	I32 getglobalNum();
  	void setPos(const IVec2&pos);
	virtual bool checkCrash(const Object*obj);
	virtual void tick();	//每帧处理结束都会调用这个函数
 };

