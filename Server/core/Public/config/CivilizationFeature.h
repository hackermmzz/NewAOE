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

//基础资源
constexpr I32 BasicStartResource[4]={200,200,150,0};
//特色文明基础资源量
constexpr std::array<std::pair<I32,std::pair<I32,I32>>, 1> FeatureStartResource={{ {CIVILIZATION_SHANG, {RESOURCESORT_FOOD,160}} }};

