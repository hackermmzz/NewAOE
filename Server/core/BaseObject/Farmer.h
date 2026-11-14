#pragma once

#include "Server/core/BaseObject/Human.h"
#include <vector>
using namespace std;

class Farmer:public Human
{
public:
	I32 resource;		                      //当前持有的资源数量
	I32 resourceMax;	                    //可以持有的资源最大数量
	I32 resourceSort;	                    //资源类型
	I32 quantity_GatherOnce;		          //资源采摘速度
	vector<SafePtr<Human>>humanTransport;	//运输的人	
public:
    Farmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type);
    I32 getResourceNowHave();           //获取当前持有的资源数量
    I32 getResourceHaveMax();           //获取可以持有的资源最大数量
    I32 getResourceHaveAddition();      //获取额外的资源采集加成
    I32 getResourceSort();              //获取资源类型
    I32 getQuantityGather();            //获取资源采摘速度
    bool isFullBackpack();              //是否采摘满了
    void addResource(I32 quantity,I32 resourceSort);      //增加手持的资源
    void transportHuman(SafePtr<Human> human);            //运输人员
  };
