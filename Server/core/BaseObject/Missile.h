#pragma once


#include "Server/core/BaseObject/MoveObject.h"
#include "Server/core/BaseObject/Attacker.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Module/Development/development.h"
#include "Server/core/BaseObject/Object.h"

class Missile : public MoveObject,public Attacker
{
protected:
	WeakPtr<Object>attacker;
	WeakPtr<Object>attackee;
	WeakPtr<Development> playerScience;
public:
    Missile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack);


};
