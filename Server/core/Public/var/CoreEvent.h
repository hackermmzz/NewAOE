#pragma once

/********************************整体任务类型************************/
enum CoreEvenType {
    CoreEven_JustMoveTo = 0,
    CoreEven_CreateBuilding,
    CoreEven_Gather,
    CoreEven_Attacking,
    CoreEven_PinPoint_Attacking,
    CoreEven_FixBuilding,
    CoreEven_BuildingAct,
    CoreEven_MissileAttack,
    CoreEven_Transport,
    CoreEven_UnLoad
};


/********************************任务细节***********************/
enum CoreDetailType {
    CoreDetail_JumpPhase = -3,
    CoreDetail_AbsoluteEnd,
    CoreDetail_NormalEnd,
    CoreDetail_Move,
    CoreDetail_Attack,
    CoreDetail_Gather,
    CoreDetail_ResourceIn,
    CoreDetail_Transport,
    CoreDetail_UpdateRatio,
    CoreDetail_Unload,
    CoreDetail_PinPoint_Attack,
};
