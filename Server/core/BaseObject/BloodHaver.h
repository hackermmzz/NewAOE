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
	//未显式初始化时保持 0，避免派生类构造顺序里读到随机内存
	BloodHaver()
		: maxBlood(Double::Zero())
		, currentBlood(Double::Zero())
		, defenceClose(Double::Zero())
		, defenceShoot(Double::Zero())
	{
	}
	//供外部读取血量。建筑在建成前需要按建造进度实时折算，因此设为虚函数。
	virtual Double getMaxBlood();
	virtual Double getBlood();
	Double getDefence(I32 attackType);
	bool isDie();
	void beHeal(Double heal);
	void beAttcked(Double damage);
	Double getDefenceCloseBasic();
	Double getDefenceShootBasic();
	Double getDefenceCloseAddition();
	Double getDefenceShootAddition();
	//血量上限变化后同步当前血量，保持「缺口」不变：
	//新当前血量 = 新上限 - (旧上限 - 旧当前血量)，即上限涨多少、当前血量也涨多少，
	//不会直接把残血单位给满上。调用前请先改好 maxBlood，再把旧值传进来。
	void syncBloodAfterMaxChange(Double oldMaxBlood, Double oldCurrentBlood);
	virtual void notifyAttacked();	//当被攻击时调用这个函数
};
