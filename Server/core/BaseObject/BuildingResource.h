#pragma once


#include "Server/core/BaseObject/Building.h"
#include "Server/core/BaseObject/Resource.h"


class BuildingResource : public Building,public Resource
{
protected:

public:
    BuildingResource(I32 type, IVec2 blockPos, WeakPtr<Development> playerScience,I32 Percent);
    Double getMaxCnt(WeakPtr<Development> playerScience);
    bool isGatherAble();                //没修建完之前不可采集
};


