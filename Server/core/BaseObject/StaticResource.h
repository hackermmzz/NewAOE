#pragma once

#include "Server/core/BaseObject/Object.h"
#include "Server/core/BaseObject/Resource.h"

class StaticRes:public Object,public Resource
{
protected:
	
public:
    StaticRes(I32 type,IVec2 detailPos);
};


