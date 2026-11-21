#pragma once

#include "Public/type/type.h"
#include "Server/conf.h"
#include "Server/core/type.h"


//游戏地图大小
constexpr U32 MAP_L=128;
constexpr U32 MAP_U=128;

//游戏1倍速情况下跑多少帧
constexpr I32 GAME_TPS=25;

//一块所占的细节坐标长宽
constexpr Double BLOCKSIDELENGTH=Double(1000);

//贴身攻击距离（区块）：攻击范围为 1 时，必须和对方相邻才能打到
constexpr I32 DISTANCE_ATTACK_CLOSE=1;

//一次攻击从出手到真正命中要经过的帧数（半秒）
constexpr I32 ATTACK_CIRCLE_FRAMES=GAME_TPS/2;

//农民/渔船的基础携带量，科技的额外携带量由 Development::get_addition_ResourceCarry 叠加
constexpr I32 FARMER_DEFAULT_CARRY=10;

