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
    //各单位初始血量
    static constexpr I32 BuildingBasicBlood[14]={};
    static constexpr I32 FarmerBasicBlood[3]={};
    static constexpr I32 AnimalBasicBlood[4]={};
    static constexpr I32 ArmyBasicBlood[15]={};
    static constexpr const I32* BasicBlood[7]={BuildingBasicBlood,nullptr,FarmerBasicBlood,AnimalBasicBlood,nullptr,BuildingBasicBlood,ArmyBasicBlood};
    
};