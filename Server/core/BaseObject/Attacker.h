#pragma once

#include "Server/core/type.h"

class Attacker{
protected:
	Double attackClose;			//近战伤害
	Double attackShoot;			//远程伤害
	I32 attackType;				//攻击类型
	bool range;					//是否为范围伤害
	I32 rangeEffect;			//范围伤害影响的范围
	I32 minAttackRange;			//攻击范围
	I32 attackCircle;			//多少帧后真正发动攻击
	I32 attackCurrentTick;		//当前进行到多少帧了
	I32 attackInteval;			//攻击间隔
public:
	Double getAttack();			//获取攻击力（包含附赠伤害)
	virtual Double getAttackAdditon();	//获取附赠伤害（不包含基础伤害)，默认 0，科技加成由 update() 烘进字段
	virtual Double getAttackBasic();	//表里的基础伤害（不含科技），由 Human/Building 覆盖
	virtual void notifyHit();	//打中了造成了伤害时候调用这个函数
	I32 getAttackCircle();		//获取多少帧后真正发动攻击,造成伤害
	I32 getCurrentAttackTick(); //获取当前进行到多少帧了
	I32 getAttackInteval();		//获取攻击间隔
	void resetAttackTick(); 	//重置攻击帧数
}; 
