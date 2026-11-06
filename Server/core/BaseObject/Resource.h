#pragma once


#include "Server/core/type.h"

class Resource
{
protected:
	Double cntLeft; //Cnt表示当前剩余的实际资源量
	Double maxCnt;
	Double decayRate;//腐烂速度
	I32 resourceSort;
	bool gatherAble;	//是否可采集
public:
	virtual bool isGatherAble();	//是否可采集，比如动物死亡才可以采集类似这样，那默认刚开始就不可以采集
	I32 getResourceSort();
	Double getCntLeft();
	Double getMaxCnt();
    void decayResource();	//腐烂资源
	bool isSurplus();	//资源是否还有剩余
};

