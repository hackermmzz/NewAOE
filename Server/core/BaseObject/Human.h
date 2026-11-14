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
    bool getTransported() const;
    void setTransported(bool value);
    //按当前科技重算血量/速度/攻防/射程等缓存值
    void refreshByScience() override;
    //兵种大类（步兵/弓兵/骑兵/祭司/攻城/船），科技查询攻防加成时要用；
    //非军队单位（农民）没有大类，统一按步兵处理。
    virtual I32 getArmyClass() const;
};

