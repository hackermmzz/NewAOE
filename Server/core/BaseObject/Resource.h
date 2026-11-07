#pragma once


#include "Server/core/type.h"

class Resource
{
protected:
	Double cntLeft; 	//当前剩余的实际资源量
	Double maxCnt;		//该资源最大量
	Double decayRate;	//腐烂速度
	I32    resourceSort;//资源种类
	bool   gatherAble;	//是否可采集
public:
	virtual bool isGatherAble();	//是否可采集，比如动物死亡才可以采集，农田要建成后才可采集。类似这样，那默认刚开始就不可以采集
	I32 getResourceSort();
	Double getCntLeft();
	Double getMaxCnt();
    void decayResource();			//腐烂资源
	bool isSurplus();				//资源是否还有剩余
};

