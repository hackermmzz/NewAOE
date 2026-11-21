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
    //设置建造进度（0~100），超过 100 按 100 计
    void build(I32 percent);
    //建成前按「满血值 × 建造进度」实时折算返回，建成后直接返回内部值
    Double getMaxBlood() override;
    Double getBlood() override;
    //UI 显示用：基础值取 ObejctBaseValue，加成值 = 当前值 - 基础值
    Double getAttackBasic() override;
    Double getAttackAdditon() override;
    Double getDefenceCloseBasic() override;
    Double getDefenceShootBasic() override;
    Double getDefenceCloseAddition() override;
    Double getDefenceShootAddition() override;

    void refresh() override;
    void refreshBlood() override;
    void refreshDefence() override;
    void refreshAttack() override;
protected:
    //100% 状态下的满血值 = 基础血量 × 血量加成 + 额外血量加成（结果为 0 时保底 1）
    Double computeFullBlood();
 };
