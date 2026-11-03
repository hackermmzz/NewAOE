#pragma once


#include "Server/core/BaseObject/Building.h"
#include "Server/core/BaseObject/Resource.h"


class BuildingResource : public Building,public Resource
{
protected:
	WeakPtr<Development> playerScience;
public:
    BuildingResource(I32 type, IVec2 blockPos, WeakPtr<Development> playerScience , I32 playerRepresent ,I32 Percent);
};


