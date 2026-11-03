#pragma once


#include "Server/core/type.h"

//BloodHaver类,用于管理一切与血量有关的数据,数据变化方式的定义

class BloodHaver
{
protected:
	Double maxBlood;	//最大血量
	Double currentBlood;//当前血量
	Double defenceClose;//近战防御
	Double defenceShoot;//远程防御
public:
	Double getMaxBlood();
	Double getBlood();
	Double getDefence(Double attackType);
	bool isDie();
	void beHeal(Double heal);
	void beAttcked(Double damage);
	Double getDefenceCloseBasic();
	Double getDefenceShootBasic();
	Double getDefenceCloseAddition();
	Double getDefenceShootAddition();
	virtual void notifyAttacked();	//当被攻击时调用这个函数
};
