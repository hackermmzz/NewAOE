#include "Server/core/Public/struct/instruction.h"
#include "Server/core/Public/var/CoreEvent.h"

CoreTask::CoreTask()
{
    insID=0;
    detailPos={0,0};
    coreEvent=0;
    worker=goal=nullptr;
    action=0;
}