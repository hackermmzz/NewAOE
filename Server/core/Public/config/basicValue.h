#pragma once


#include <vector>
#include <array>
#include <utility>
#include <map>
#include "Public/type/type.h"
#include "Server/conf.h"
#include "Server/core/type.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Sort.h"

struct __GameBasicValue{
    //开局初始资源
    static constexpr I32 BasicStartResource[4]={200,200,150,0};
    
};