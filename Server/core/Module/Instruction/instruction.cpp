#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <limits>

namespace aoe {
using I32=std::int32_t; using U64=std::uint64_t;
template<class T> using SafePtr=std::shared_ptr<T>;
template<class T> using WeakPtr=std::weak_ptr<T>;
struct IVec2 { I32 x=0,y=0; };
// Integrate these definitions with Public/var and Public/config in the host project.
constexpr I32 GAME_TPS=25;
enum ACTION_STATUS_CODE {
 ACTION_SUCCESS=0, ACTION_INVALID_HUMANBUILD_DIFFERENTHIGH,
 ACTION_INVALID_HUMANBUILD_OVERBORDER, ACTION_INVALID_HUMANBUILD_UNEXPLORE,
 ACTION_INVALID_HUMANBUILD_OVERLAP, ACTION_INVALID_HUMANBUILD_LOCK,
 ACTION_INVALID_DISTANCE_FAR, ACTION_INVALID_FULLY_LOAD,
 ACTION_INVALID_HUMANACTION_BUILDRESOURCENOMATCH,
 ACTION_INVALID_HUMANACTION_BUILDNOTNEEDFIX, ACTION_INVALID_BUILDACT_MAXHUMAN,
 ACTION_INVALID_BUILDACT_LOCK, ACTION_INVALID_BUILDACT_NEEDBUILT,
 ACTION_INVALID_NULLGOALOBJECT, ACTION_INVALID_NULLWORKER,
 ACTION_INVALID_PINPOINT_NOT_FIT, ACTION_INVALID_PRIEST_TARGET_ERROR,
 ACTION_INVALID_UPGRADE_TIME, ACTION_INVALID_RESOURCE, ACTION_INVALID_LOCATION,
 ACTION_INVALID_WORKER_BUSY, ACTION_INVALID_ACTION
};
enum Event : I32 { Move, Attack, Gather, Repair, Transport, Unload,
 PinpointAttack, MissileAttack, CreateBuilding, BuildingAction, Heal };
enum class Kind { Farmer, Soldier, Priest, FishingBoat, TransportBoat, Building, Resource, Animal };
// Minimal integration view; no ownership of objects from other modules.
struct Object {
 I32 sn=0,player=0; Kind kind=Kind::Farmer;
 bool alive=true,land=true,constructed=true,finished=true,fullHp=false;
 bool canAttack=false,canPinpoint=false; I32 carried=0,capacity=5;
 I32 resourceType=0,gatherSource=-1; bool backpackEmpty=true;
 virtual ~Object()=default;
};
struct Instruction {
 U64 insID=0; IVec2 detailPos; I32 coreEvent=Move,workerSN=0,goalSN=0,action=0;
};
using tagInstruction=Instruction;
struct CoreProcessTack {
 U64 insID=0; IVec2 detailPos; I32 coreEvent=Move;
 WeakPtr<Object> worker,goal; I32 action=0;
 bool clearBackpack=false; // Applied by the kernel only after accepting the task.
};
using CoreTask=CoreProcessTack;
struct BuildingSpec { I32 width=0,height=0; };
// The host must implement all policy checks using its authoritative data.
// Check methods must be read-only and return the exact ACTION_STATUS_CODE.
struct Context {
 virtual ~Context()=default;
 virtual WeakPtr<Object> find(I32 sn) const=0;
 virtual I32 mapWidth() const=0; virtual I32 mapHeight() const=0;
 virtual I32 blockSide() const=0;
 virtual bool ocean(IVec2 block) const=0;
 virtual bool explored(I32 player,IVec2 block) const=0;
 virtual I32 height(IVec2 block) const=0;
 virtual bool blocked(IVec2 block) const=0;
 virtual BuildingSpec buildingSpec(I32 type) const=0;
 // Includes civilization/stage unlock and resource requirements.
 virtual I32 checkBuild(const Object&,I32 type) const=0;
 // Includes unlock, population (including queued units), resources and stage conditions.
 virtual I32 checkBuildingAction(const Object&,I32 action) const=0;
 // Includes gather compatibility/surplus, warehouse matching, ownership/team,
 // attack legality, priest conversion, transport distance/capacity, etc.
 virtual I32 checkInteraction(const Object&,const Object&,I32 event) const=0;
 virtual void result(U64 insID,I32 code)=0;
};
class InstructionModule {
 Context& ctx; I32 tps;
 std::unordered_map<I32,I32> lastFrame;
 std::vector<CoreTask> coreTasks;
 bool inside(IVec2 p) const {
  return p.x>=0 && p.y>=0 && ctx.blockSide()>0 &&
   std::int64_t(p.x)<std::int64_t(ctx.mapWidth())*ctx.blockSide() &&
   std::int64_t(p.y)<std::int64_t(ctx.mapHeight())*ctx.blockSide();
 }
 I32 push(CoreTask task) { coreTasks.push_back(std::move(task)); return ACTION_SUCCESS; }
 static bool human(const Object& o) { return o.kind==Kind::Farmer || o.kind==Kind::Soldier || o.kind==Kind::Priest; }
public:
 explicit InstructionModule(Context& c,I32 ticks=GAME_TPS):ctx(c),tps(ticks>0?ticks:GAME_TPS) {}
 std::vector<CoreTask>& getCoreTask() noexcept { return coreTasks; }
 void tick() noexcept {} // Kernel drains/clears getCoreTask after consumption.
 I32 addRelation(WeakPtr<Object> a,WeakPtr<Object> b,I32 event) {
  auto w=a.lock(); if(!w || !w->alive) return ACTION_INVALID_NULLWORKER;
  auto g=b.lock(); if(!g || !g->alive) return ACTION_INVALID_NULLGOALOBJECT;
  if(event!=Attack && event!=Gather && event!=Repair && event!=Transport && event!=Heal) return ACTION_INVALID_ACTION;
  if(w->kind==Kind::Building && !w->constructed) return ACTION_INVALID_BUILDACT_NEEDBUILT;
  if(event==Repair) {
   if(w->kind!=Kind::Farmer || g->kind!=Kind::Building) return ACTION_INVALID_ACTION;
   if(g->fullHp && g->constructed) return ACTION_INVALID_HUMANACTION_BUILDNOTNEEDFIX;
  }
  if(event==Gather && w->kind!=Kind::Farmer && w->kind!=Kind::FishingBoat) return ACTION_INVALID_ACTION;
  if(event==Attack && w->kind==Kind::Priest && w->player==g->player) event=Heal;
  if(event==Heal && (w->kind!=Kind::Priest || w.get()==g.get() || !human(*g) || !g->land || g->fullHp || w->player!=g->player)) return ACTION_INVALID_PRIEST_TARGET_ERROR;
  if(event==Attack && !w->canAttack && w->kind!=Kind::Priest) return ACTION_INVALID_ACTION;
  if(event==Transport) {
   if(!human(*w) || g->kind!=Kind::TransportBoat) return ACTION_INVALID_ACTION;
   if(g->carried>=g->capacity) return ACTION_INVALID_FULLY_LOAD;
  }
  auto code=ctx.checkInteraction(*w,*g,event); if(code!=ACTION_SUCCESS) return code;
  CoreTask task{}; task.worker=a; task.goal=b; task.coreEvent=event;
  task.clearBackpack=event==Gather && w->kind==Kind::Farmer && !w->backpackEmpty && g->gatherSource>=0 && w->gatherSource!=g->gatherSource;
  return push(task);
 }
 I32 addRelation(WeakPtr<Object> a,IVec2 p,I32 event) {
  auto w=a.lock(); if(!w || !w->alive) return ACTION_INVALID_NULLWORKER;
  const bool pinpoint=event==PinpointAttack || event==MissileAttack;
  if(event!=Move && event!=Unload && !pinpoint) return ACTION_INVALID_ACTION;
  if(!inside(p)) return pinpoint?ACTION_INVALID_PINPOINT_NOT_FIT:ACTION_INVALID_LOCATION;
  if(w->kind==Kind::Building || w->kind==Kind::Resource || w->kind==Kind::Animal) return ACTION_INVALID_ACTION;
  if(pinpoint && !w->canPinpoint) return ACTION_INVALID_ACTION;
  if(event==Unload && w->kind!=Kind::TransportBoat) return ACTION_INVALID_ACTION;
  if(event==Move && w->kind==Kind::TransportBoat && !ctx.ocean({p.x/ctx.blockSide(),p.y/ctx.blockSide()})) event=Unload;
  CoreTask task{}; task.worker=a; task.detailPos=p; task.coreEvent=event; return push(task);
 }
 I32 addRelation(WeakPtr<Object> a,IVec2 p,I32 event,I32 type) {
  auto w=a.lock(); if(!w || !w->alive) return ACTION_INVALID_NULLWORKER;
  if(event!=CreateBuilding || w->kind!=Kind::Farmer) return ACTION_INVALID_ACTION;
  auto spec=ctx.buildingSpec(type);
  if(spec.width<=0 || spec.height<=0) return ACTION_INVALID_ACTION;
  if(p.x<0 || p.y<0 || std::int64_t(p.x)+spec.width>ctx.mapWidth() || std::int64_t(p.y)+spec.height>ctx.mapHeight()) return ACTION_INVALID_HUMANBUILD_OVERBORDER;
  auto code=ctx.checkBuild(*w,type); if(code!=ACTION_SUCCESS) return code;
  const auto h=ctx.height(p);
  for(I32 x=p.x;x<std::int64_t(p.x)+spec.width;++x) for(I32 y=p.y;y<std::int64_t(p.y)+spec.height;++y) {
   IVec2 cell{x,y};
   if(!ctx.explored(w->player,cell)) return ACTION_INVALID_HUMANBUILD_UNEXPLORE;
   if(ctx.height(cell)!=h) return ACTION_INVALID_HUMANBUILD_DIFFERENTHIGH;
   if(ctx.blocked(cell)) return ACTION_INVALID_HUMANBUILD_OVERLAP;
  }
  CoreTask task{}; task.worker=a; task.coreEvent=event; task.action=type;
  // For CreateBuilding only, detailPos carries block coordinates, as in the specified overload.
  task.detailPos=p; return push(task);
 }
 I32 addRelation(WeakPtr<Object> a,I32 event,I32 action) {
  auto w=a.lock(); if(!w || !w->alive) return ACTION_INVALID_NULLWORKER;
  if(event!=BuildingAction || w->kind!=Kind::Building) return ACTION_INVALID_ACTION;
  if(!w->constructed || !w->finished) return ACTION_INVALID_BUILDACT_NEEDBUILT;
  auto code=ctx.checkBuildingAction(*w,action); if(code!=ACTION_SUCCESS) return code;
  CoreTask task{}; task.worker=a; task.coreEvent=event; task.action=action; return push(task);
 }
 void filterInstruction(std::vector<tagInstruction>& ins,I32 coreClock) {
  std::vector<tagInstruction> accepted; accepted.reserve(ins.size());
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
     case Move: case Unload: case PinpointAttack: case MissileAttack: code=addRelation(w,i.detailPos,i.coreEvent); break;
     case Attack: case Gather: case Repair: case Transport: case Heal: code=addRelation(w,ctx.find(i.goalSN),i.coreEvent); break;
     case CreateBuilding: code=addRelation(w,i.detailPos,i.coreEvent,i.action); break;
     case BuildingAction: code=addRelation(w,i.coreEvent,i.action); break;
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
 void resetClock() { lastFrame.clear(); }
};
} // namespace aoe
