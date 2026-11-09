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
    void setPercent(I32 percent);   //推进建造进度，到 100 自动标记完成
    I32 getPercent();               //当前建造进度 0~100
    bool isMatchResourceType(int productSort); //判断是否能收集某个类型的物资
    void setPlayerScience( WeakPtr<Development> science ){ this->playerScience = science; }
    WeakPtr<Development> getPlayerScience(){return playerScience;}
    virtual void update();	//重算血量/攻防/射程
    //下面 6 个把"表里的基础值"和"实际值减基础值"分开，UI 要分别显示
    Double getAttackBasic() override;
    Double getAttackAdditon() override;
    Double getDefenceCloseBasic() override;
    Double getDefenceShootBasic() override;
    Double getDefenceCloseAddition() override;
    Double getDefenceShootAddition() override;
 };
