#pragma once

#include "Server/core/BaseObject/Human.h"
#include <vector>
using namespace std;

class Farmer:public Human
{
public:
	I32 resource;		//当前持有的资源数量
	I32 resourceMax;	//可以持有的资源最大数量
	I32 resourceSort;	//资源类型
	I32 quantity_GatherOnce;		//资源采摘速度
	vector<SafePtr<Human>>humanTransport;	//运输的人	
public:
    Farmer(IVec2 detailPos , WeakPtr<Development> playerScience , I32 playerRepresent,I32 type);
    I32 getResourceNowHave();
    I32 getResourceHaveMax();
    I32 getResourceHaveAddition();//
    I32 getResourceSort();
    I32 getQuantityGather();
    bool isFullBackpack();//是否采摘满了
    void addResource(I32 quantity,I32 resourceSort);
    void transportHuman(SafePtr<Human> human);
  };
