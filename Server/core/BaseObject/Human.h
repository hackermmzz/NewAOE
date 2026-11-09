#pragma once

#include "Server/core/BaseObject/MoveObject.h"
#include "Server/core/BaseObject/Resource.h"
#include "Server/core/BaseObject/BloodHaver.h"
#include "Server/core/BaseObject/Attacker.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Module/Development/development.h"
#include "Public/vec/vec.hpp"
#include "Server/core/Public/config/Core.h"
#include "Server/core/Public/var/Sort.h"

class Human:public MoveObject,public BloodHaver,public Attacker
{
protected:
	 WeakPtr<Development> playerScience;	//科技
	 bool transported;	                    //是否被运输
public:
    Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience);
    void setPlayerScience( WeakPtr<Development> science ){ this->playerScience = science; }
    WeakPtr<Development> getPlayerScience(){return playerScience;}
    virtual void update();	//科技变化后重算与科技有关的数值（血量/速度）
    //下面 6 个把"表里的基础值"和"实际值减基础值"分开，UI 要分别显示
    Double getAttackBasic() override;
    Double getAttackAdditon() override;
    Double getDefenceCloseBasic() override;
    Double getDefenceShootBasic() override;
    Double getDefenceCloseAddition() override;
    Double getDefenceShootAddition() override;
    bool getTransported() const;
    void setTransported(bool value);
};

