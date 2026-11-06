#pragma once

#include "Server/core/BaseObject/MoveObject.h"
#include "Server/core/BaseObject/Resource.h"
#include "Server/core/BaseObject/BloodHaver.h"
#include "Server/core/BaseObject/Attacker.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Module/Development/development.h"
#include "Public/vec/vec.hpp"
#include "Server/core/Public/var/Sort.h"

class Human:public MoveObject,public BloodHaver,public Attacker
{
protected:
	 WeakPtr<Development> playerScience;	//科技
	 bool transported;	//是否被运输
public:
    Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience, I32 playerRepresent);
    void setPlayerScience( WeakPtr<Development> science ){ this->playerScience = science; }
    bool getTransported() const;
    void setTransported(bool value);
};

