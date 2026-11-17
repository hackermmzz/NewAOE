#pragma once

#include <string>
#include "Public/vec/vec.hpp"
#include "Server/core/BaseObject/CrashBox.h"
#include "Server/core/Public/config/ObjectBaseValue.h"
using namespace std;

class Object
{
protected:
	I32 playerID;						//所属玩家的编号（如果属于公立资源，那么为-1）
	I32 teamID;	 						//所属团队的编号
	//WeakPtr<Player> player;			//所属玩家//废弃，player内部拿不到自己的WeakPtr
	I32 sort;							//对象的大类型，比如农民，军队等
	I32 type;							//对象具体类型(比如运输船、农民、投石兵等这些具体角色）
	U64 SN;								//全局对象编号
	string name;						//对象名称
	IVec2 detail_pos;					//细节坐标
	IVec2 block_pos;					//区块坐标
	I32 vision;							//视野范围
	union{
		CrashBox_Rect crashBoxRect;		//矩形碰撞包围盒
		CrashBox_Circle crashBoxCircle; //圆形碰撞包围盒
	};
	static U64 globalSNIndex;			//给全局对象分配SN
public:
	Object();
	I32 getSort();
	I32 getType();
    //I32 getPlayerRepresent();
	//void setPlayerRepresent(I32 playerRepresent);
	IVec2 getBlockPos();
	IVec2 getDetailPos();
	string getName();
	I32 getVision();
  	I32 getglobalNum();
  	void setPos(const IVec2&pos);
	//改变具体类型（兵种/建筑升级用），同时刷新名称、视野、碰撞盒与科技相关数值。
	//sort 不变，只换 type。
	void setType(I32 newType);
	//void setplayer(WeakPtr<Player> player);
	//void getplayer(WeakPtr<Player> player);
	void setBelong(I32 playerID,I32 teamID);		//设置对象所属玩家和团队（只需要设一个的话，另一个填成负数就行）
	std::pair<I32,I32> getBelong();					//获取对象所属玩家和团队（first为playerID，second为teamID）
	virtual bool checkCrash(const Object*obj);
	virtual void tick();							//每帧处理结束都会调用这个函数
	//科技变化后按「基础值 × 科技加成」重算缓存数值。默认没有依赖科技的缓存值。
	//由 Player::notifyBuildingActionFinish 在建筑动作（科技/解锁类）完成时统一触发。
	virtual void refreshByScience();
protected:
	//派生类把 sort / type / detail_pos 设好之后调用，按基础数值表建立碰撞盒。
	//联合体本身不记录当前存的是圆还是矩形，所以一律以数值表的口径为准。
	void rebuildCrashBox();
 };
