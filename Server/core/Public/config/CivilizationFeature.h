#pragma once

#include "basicValue.h"

class GameValue{
    private:
    //特色文明开局资源量
    static constexpr pair<I32,std::pair<I32,I32>> FeatureStartResource[1]={ {CIVILIZATION_SHANG, {RESOURCESORT_FOOD,160} }};

    public:
    //获取开局初始资源
    static std::array<I32, 4> getStartResource(I32 civilization){
        array<I32, 4> tmp;
        for(I32 i=0;i<4;i++){
            tmp[i] = __GameBasicValue::BasicStartResource[i];
        }
        return tmp;
    }


};
