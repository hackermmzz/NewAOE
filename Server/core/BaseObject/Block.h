#pragma once

#include "Public/vec/vec.hpp"

struct Block
{
    IVec2 blockpos;
    I32 blockType;	//block的类型，比如海洋，陆地，目前就这2种
    I32 blockHeight;	//block的高度，通常只有陆地才需要设置高度
};
