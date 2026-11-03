#pragma once

#include "public/vec/vec.hpp"
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/BaseObject/Object.h"

struct Instruction{
    U64   insID;    //指令的ID
    IVec2 detailPos;    //细节坐标
    I32 coreEvent;  //任务类型
    I32 workerSN;   //woker的SN
    I32 goalSN; //goal的SN
    I32 action;    //建筑要干的任务 
};



struct CoreTask{
    U64   insID;    //对应的指令ID
    IVec2 detailPos;    //细节坐标
    I32 coreEvent;  //内核任务
    WeakPtr<Object> worker; //
    WeakPtr<Object> goal;   //
    I32 action;
    CoreTask();
};


