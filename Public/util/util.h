#pragma once

#include "Public/type/type.h"
#include <string>
#include <cassert>
#include<functional>

#define Assert(status,msg) do{assert( (!(status)) && msg );}while(false)


U64 GetTimeMS();

std::string UUID();

bool wait_for(U32 seconds,const std::function<bool()>&cond);