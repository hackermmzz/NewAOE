#pragma once

#include "Server/core/BaseObject/Human.h"

class Army : public Human
{
protected:
	WeakPtr<Development> playerScience;
	I32 armyClass;//军队的类别，比如骑兵，近战兵，远战兵
public:
    Army(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience , I32 playerRepresent);
};
