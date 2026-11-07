#pragma once

#include "Server/core/BaseObject/Object.h"
#include "Server/core/BaseObject/BloodHaver.h"
#include "Server/core/BaseObject/Attacker.h"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/Public/config/Core.h"
#include "Server/core/Module/Development/development.h"
#include "Server/core/Public/var/Building.h"
#include "Server/core/Public/var/Sort.h"

class Building:public Object,public BloodHaver,public Attacker
{
protected:
	WeakPtr<Development> playerScience;
	I32 fundation;	        //地基类型
	bool constructed;	    //是否建造完成
public:
    Building(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience);
    bool isConstructed();   //判断已建造完成
    bool isMatchResourceType(int productSort); //判断是否能收集某个类型的物资
 };
