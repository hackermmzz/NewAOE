#pragma once

#include "Server/core/BaseObject/Object.h"
#include "Server/core/type.h"
#include <stack>
using  namespace std;

class MoveObject:public Object
{
protected:
	Double 			speed;   			//移动速度
	stack<IVec2> 	path;    			//记录路径
	IVec2 			targetDetailPos;	//目的地细节坐标
public:
    void setPath(const stack<IVec2>&path,const IVec2&targetDetailPoint);	//设置寻路路径
	FORCE_INLINE IVec2 	nextPos(){
		if(!path.empty()){
			IVec2 next = path.top();
			path.pop();
			return next;
		}
		return targetDetailPos;
	}
	FORCE_INLINE IVec2 	getTargetDetailPos(){return targetDetailPos;	/*获取最终的目标点*/}
	virtual Double		getSpeed();				//获取速度
	virtual void 		refreshSpeed();
};

