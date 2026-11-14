#pragma once

#include "Server/core/BaseObject/Object.h"
#include "Server/core/BaseObject/BloodHaver.h"
#include "Server/core/BaseObject/Attacker.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Public/config/Core.h"
#include "Server/core/Module/Development/development.h"
#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Sort.h"

class Building:public Object,public BloodHaver,public Attacker
{
protected:
	WeakPtr<Development> playerScience;
	I32 fundation;	        //地基类型
	bool constructed;	    //是否建造完成
    I32 Percent;
public:
    Building(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience,I32 percent);
    bool isConstructed();   //判断已建造完成
    bool isMatchResourceType(int productSort); //判断是否能收集某个类型的物资
    void setPlayerScience( WeakPtr<Development> science ){ this->playerScience = science; }
    WeakPtr<Development> getPlayerScience(){return playerScience;}
    //按当前科技重算血量/攻防/射程等缓存值
    void refreshByScience() override;
protected:
    //(基础血量 × 血量加成 + 额外血量加成) × 建造完成度；未建成时按 Percent 折算，
    //建成后不再折算。算出来是 0（Percent 为 0）时强制给 1 点，避免建筑一落地就是死的。
    Double computeMaxBlood();
 };
