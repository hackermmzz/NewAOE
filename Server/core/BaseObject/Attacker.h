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
	//所有攻击字段先清零，真实数值由派生类按「基础数值表 + 科技」填进来
	Attacker();
public:
	Double getAttack();			//获取攻击力（基础 + 科技加成）
	//基础值 / 加成值分开给，供 UI 显示。
	//基础值来自 ObejctBaseValue，由带科技的对象（Human/Building）覆写；
	//默认实现直接返回内部值，适配没有科技加成的对象（动物、投掷物）。
	virtual Double getAttackBasic();	//基础伤害
	virtual Double getAttackAdditon();	//科技加成值 = 当前攻击力 - 基础值
	virtual void notifyHit();	//打中了造成了伤害时候调用这个函数
	virtual I32 getAttackCircle();		//获取多少帧后真正发动攻击,造成伤害
	I32 getCurrentAttackTick(); //获取当前进行到多少帧了
	virtual I32 getAttackInteval();		//获取攻击间隔
	void resetAttackTick(); 	//重置攻击帧数
}; 
