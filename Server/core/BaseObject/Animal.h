#pragma once

#include "Public/type/type.h"
#include "Public/vec/vec.hpp"
#include "Server/core/BaseObject/MoveObject.h"
#include "Server/core/BaseObject/Resource.h"
#include "Server/core/BaseObject/BloodHaver.h"

class Animal:public MoveObject, public Resource, public BloodHaver
{
protected:
	I32 friendly;	//友好程度

public:
	Animal(I32 type,IVec2 detailPos);
	I32 getFriendly();	
};
