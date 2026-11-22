#include "Server/core/Module/Instruction/instruction.h"

bool InstructionModule::inside(IVec2 p) const
{
  return p.x>=0 && p.y>=0 && ctx.blockSide()>0 &&
   std::int64_t(p.x)<std::int64_t(ctx.mapWidth())*ctx.blockSide() &&
   std::int64_t(p.y)<std::int64_t(ctx.mapHeight())*ctx.blockSide();
 }

I32 InstructionModule::push(CoreTask task)
{ coreTasks.push_back(std::move(task)); return ACTION_SUCCESS; }

bool InstructionModule::isVillager(Object& o)
{ return o.getSort()==SORT_FARMER && o.getType()==FARMERTYPE_FARMER; }

bool InstructionModule::isArmy(Object& o)
{ return o.getSort()==SORT_ARMY; }

bool InstructionModule::isPriest(Object& o)
{ return isArmy(o) && o.getType()==AT_PRIEST; }

bool InstructionModule::isFishingBoat(Object& o)
{
    return o.getSort() == SORT_FARMER &&
           (o.getType() == FARMERTYPE_SMALL_SAILING || o.getType() == FARMERTYPE_BIG_SAILING);
}

bool InstructionModule::isTransportBoat(Object& o)
{
    return o.getSort() == SORT_FARMER &&
           (o.getType() == FARMERTYPE_LIGHT_TRANSPORT || o.getType() == FARMERTYPE_HEAVY_TRANSPORT);
}

bool InstructionModule::isBuilding(Object& o)
{ return o.getSort()==SORT_BUILDING || o.getSort()==SORT_BUILDING_RESOURCE; }

bool InstructionModule::isAnimal(Object& o)
{ return o.getSort()==SORT_ANIMAL; }

bool InstructionModule::isStaticResource(Object& o)
{ return o.getSort()==SORT_STATICRES; }

bool InstructionModule::isShip(Object& o)
{
    if (o.getSort() == SORT_FARMER) {
        return isFishingBoat(o) || isTransportBoat(o) ||
               o.getType() == FARMERTYPE_SMALL_TRADE_BOAT || o.getType() == FARMERTYPE_BIG_TRADE_BOAT;
    }
    if (!isArmy(o)) return false;
    switch (o.getType()) {
        case AT_SCOUT_SHIP:
        case AT_WAR_GALLEY:
        case AT_TRIREME:
        case AT_CATAPULT_TRIREME:
        case AT_FIRESAILBOAT:
            return true;
        default:
            return false;
    }
}

bool InstructionModule::human(Object& o)
{ return isVillager(o) || (isArmy(o) && !isShip(o)); }

InstructionModule::InstructionModule(Context& c,I32 ticks):ctx(c),tps(ticks>0?ticks:GAME_TPS)
{}

std::vector<CoreTask>& InstructionModule::getCoreTask() noexcept
{ return coreTasks; }

void InstructionModule::tick() noexcept
{}

I32 InstructionModule::addRelation(WeakPtr<Object> a,WeakPtr<Object> b,I32 event)
{
  auto w=a.lock(); if(!w || !ctx.alive(*w)) return ACTION_INVALID_NULLWORKER;
  auto g=b.lock(); if(!g) return ACTION_INVALID_NULLGOALOBJECT;
  // A dead animal can still be a gatherable resource. The adapter checks isGatherAble().
  if(!ctx.alive(*g) && !(event==CoreEven_Gather && isAnimal(*g))) return ACTION_INVALID_NULLGOALOBJECT;
  if(event!=CoreEven_Attacking && event!=CoreEven_Gather && event!=CoreEven_FixBuilding && event!=CoreEven_Transport) return ACTION_INVALID_ACTION;
  if(isBuilding(*w) && !ctx.constructed(*w)) return ACTION_INVALID_BUILDACT_NEEDBUILT;
  if(event==CoreEven_FixBuilding) {
   if(!isVillager(*w) || (!isBuilding(*g) && !isShip(*g))) return ACTION_INVALID_ACTION;
   // Ship repair shore/resource rules are checked by the kernel.
   if(ctx.fullHp(*g) && (!isBuilding(*g) || ctx.constructed(*g))) return ACTION_INVALID_HUMANACTION_BUILDNOTNEEDFIX;
  }
  // Soldiers cannot gather wood, stone, gold or any other resource.
  if(event==CoreEven_Gather && !isVillager(*w) && !isFishingBoat(*w)) return ACTION_INVALID_ACTION;
  if (event == CoreEven_Attacking && isPriest(*w)) {
   const auto [workerPlayer, workerTeam] = w->getBelong();
   const auto [goalPlayer, goalTeam] = g->getBelong();
   const bool friendly = workerPlayer >= 0 && goalPlayer >= 0 &&
       (workerPlayer == goalPlayer || (workerTeam >= 0 && workerTeam == goalTeam));
   if (friendly && (w.get() == g.get() || !human(*g) || ctx.fullHp(*g))) {
    return ACTION_INVALID_PRIEST_TARGET_ERROR;
   }
  }
  if(event==CoreEven_Attacking && (isFishingBoat(*w) || isTransportBoat(*w))) return ACTION_INVALID_ACTION;
  if(event==CoreEven_Attacking && (isArmy(*w) || isPriest(*w)) && (isAnimal(*g) && g->getType()==ANIMAL_TREE)) return ACTION_INVALID_ACTION;
  if(event==CoreEven_Attacking && !ctx.canAttack(*w) && !isPriest(*w)) return ACTION_INVALID_ACTION;
  if(event==CoreEven_Transport) {
   if(!human(*w) || !isTransportBoat(*g)) return ACTION_INVALID_ACTION;
   if(ctx.carried(*g)>=ctx.capacity(*g)) return ACTION_INVALID_FULLY_LOAD;
  }
  auto code=ctx.checkInteraction(*w,*g,event); if(code!=ACTION_SUCCESS) return code;
  CoreTask task{}; task.worker=a; task.goal=b; task.coreEvent=event;
  return push(task);
 }

I32 InstructionModule::addRelation(WeakPtr<Object> a,IVec2 p,I32 event)
{
  auto w=a.lock(); if(!w || !ctx.alive(*w)) return ACTION_INVALID_NULLWORKER;
  const bool pinpoint=event==CoreEven_PinPoint_Attacking || event==CoreEven_MissileAttack;
  if(event!=CoreEven_JustMoveTo && event!=CoreEven_UnLoad && !pinpoint) return ACTION_INVALID_ACTION;
  if(!inside(p)) return pinpoint?ACTION_INVALID_PINPOINT_NOT_FIT:ACTION_INVALID_LOCATION;
  if(isBuilding(*w) || isStaticResource(*w) || isAnimal(*w)) return ACTION_INVALID_ACTION;
  if(pinpoint && (isFishingBoat(*w) || isTransportBoat(*w) || !ctx.canPinpoint(*w))) return ACTION_INVALID_ACTION;
  if(event==CoreEven_UnLoad && !isTransportBoat(*w)) return ACTION_INVALID_ACTION;
  if(event==CoreEven_JustMoveTo && isTransportBoat(*w) && !ctx.ocean({p.x/ctx.blockSide(),p.y/ctx.blockSide()})) event=CoreEven_UnLoad;
  CoreTask task{}; task.worker=a; task.detailPos=p; task.coreEvent=event; return push(task);
 }

I32 InstructionModule::addRelation(WeakPtr<Object> a,IVec2 p,I32 event,I32 type)
{
  auto w=a.lock(); if(!w || !ctx.alive(*w)) return ACTION_INVALID_NULLWORKER;
  if(event!=CoreEven_CreateBuilding || !isVillager(*w)) return ACTION_INVALID_ACTION;
  PROFILE_TRACE();
  auto spec=ctx.buildingSpec(type);
  if(spec.width<=0 || spec.height<=0) return ACTION_INVALID_ACTION;
  if(p.x<0 || p.y<0 || std::int64_t(p.x)+spec.width>ctx.mapWidth() || std::int64_t(p.y)+spec.height>ctx.mapHeight()) return ACTION_INVALID_HUMANBUILD_OVERBORDER;
  auto code=ctx.checkBuild(*w,type); if(code!=ACTION_SUCCESS) return code;
  const auto h=ctx.height(p);
  for(I32 x=p.x;x<std::int64_t(p.x)+spec.width;++x) for(I32 y=p.y;y<std::int64_t(p.y)+spec.height;++y) {
   IVec2 cell{x,y};
   if(!ctx.explored(w->getBelong().first,cell)) return ACTION_INVALID_HUMANBUILD_UNEXPLORE;
   if(!spec.isFarm && (ctx.isSlope(cell) || ctx.height(cell)!=h))
    return ACTION_INVALID_HUMANBUILD_DIFFERENTHIGH;
   if(ctx.blocked(cell)) return ACTION_INVALID_HUMANBUILD_OVERLAP;
  }
  CoreTask task{}; task.worker=a; task.coreEvent=event; task.action=type;
  // For CoreEven_CreateBuilding only, detailPos carries block coordinates, as in the specified overload.
  task.detailPos=p; return push(task);
 }

I32 InstructionModule::addRelation(WeakPtr<Object> a,I32 event,I32 action)
{
  auto w=a.lock(); if(!w || !ctx.alive(*w)) return ACTION_INVALID_NULLWORKER;
  if(event!=CoreEven_BuildingAct || !isBuilding(*w)) return ACTION_INVALID_ACTION;
  if(!ctx.constructed(*w)) return ACTION_INVALID_BUILDACT_NEEDBUILT;
  auto code=ctx.checkBuildingAction(*w,action); if(code!=ACTION_SUCCESS) return code;
  CoreTask task{}; task.worker=a; task.coreEvent=event; task.action=action; return push(task);
 }

void InstructionModule::filterInstruction(std::vector<Instruction>& ins,I32 coreClock)
{
  PROFILE_TRACE();
  std::vector<Instruction> accepted; accepted.reserve(ins.size());
  // Negative/backward clocks are invalid; call resetClock on a new match.
  for(const auto& i:ins) {
   I32 code=ACTION_INVALID_ACTION; const auto start=coreTasks.size();
   auto it=lastFrame.find(i.workerSN);
   if(coreClock<0) code=ACTION_INVALID_ACTION;
   else if(it!=lastFrame.end() && std::int64_t(coreClock)-it->second<tps) code=ACTION_INVALID_WORKER_BUSY;
   else {
    // Every first attempt consumes the rate window, including rejected attempts.
    lastFrame[i.workerSN]=coreClock;
    auto w=ctx.find(i.workerSN);
    switch(i.coreEvent) {
     case CoreEven_JustMoveTo: case CoreEven_UnLoad: case CoreEven_PinPoint_Attacking: case CoreEven_MissileAttack: code=addRelation(w,i.detailPos,i.coreEvent); break;
     case CoreEven_Attacking: case CoreEven_Gather: case CoreEven_FixBuilding: case CoreEven_Transport: code=addRelation(w,ctx.find(i.goalSN),i.coreEvent); break;
     case CoreEven_CreateBuilding: code=addRelation(w,i.detailPos,i.coreEvent,i.action); break;
     case CoreEven_BuildingAct: code=addRelation(w,i.coreEvent,i.action); break;
     default: break;
    }
   }
   if(code==ACTION_SUCCESS) {
    coreTasks[start].insID=i.insID;
    auto corrected=i; corrected.coreEvent=coreTasks[start].coreEvent; accepted.push_back(corrected);
   }
   ctx.result(i.insID,code);
  }
  ins.swap(accepted);
 }

void InstructionModule::resetClock()
{ lastFrame.clear(); }
