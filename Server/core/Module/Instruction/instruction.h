#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <utility>
#include "Server/core/BaseObject/WeakPointer.hpp"
#include "Server/core/BaseObject/Object.h"
#include "Server/core/Public/struct/instruction.h"
#include "Server/core/Public/var/CoreEvent.h"
#include "Server/core/Public/var/ErrorCode.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/var/Farmer.h"
#include "Server/core/Public/var/Army.h"
#include "Server/core/Public/var/Animal.h"
#include "Server/core/Public/config/Core.h"
#include "Public/profile/profile.h"

struct BuildingSpec {
 I32 width=0,height=0;
 bool isFarm=false; // Set only for the actual farmland building type.
};
// The host must implement all policy checks using its authoritative data.
// Check methods must be read-only and return the exact ACTION_STATUS_CODE.
struct Context {
 virtual ~Context()=default;
 virtual bool alive(Object&) const=0;
 virtual bool fullHp(Object&) const=0;
 virtual bool constructed(Object&) const=0;
 virtual bool canAttack(Object&) const=0;
 virtual bool canPinpoint(Object&) const=0;
 virtual I32 carried(Object&) const=0;
 virtual I32 capacity(Object&) const=0;
 virtual WeakPtr<Object> find(I32 sn) const=0;
 virtual I32 mapWidth() const=0; virtual I32 mapHeight() const=0;
 virtual I32 blockSide() const=0;
 virtual bool ocean(IVec2 block) const=0;
 virtual bool explored(I32 player,IVec2 block) const=0;
 virtual I32 height(IVec2 block) const=0;
 // Read the terrain slope flag/corner heights, including slopes within one block.
 virtual bool isSlope(IVec2 block) const=0;
 virtual bool blocked(IVec2 block) const=0;
 virtual BuildingSpec buildingSpec(I32 type) const=0;
 // Includes civilization/stage unlock and resource requirements.
 virtual I32 checkBuild(Object&,I32 type) const=0;
 // Includes static unlock, population, resources and stage conditions. The kernel owns running research locks and task state.
 virtual I32 checkBuildingAction(Object&,I32 action) const=0;
 // Includes gather compatibility/surplus, warehouse matching, ownership/team,
 // attack legality, priest conversion, transport distance/capacity, etc.
 // CoreEven_Gather must check Resource::isGatherAble(), including dead animals.
 // The kernel owns shore repair, refunds and research locks.
 virtual I32 checkInteraction(Object&,Object&,I32 event) const=0;
 virtual void result(U64 insID,I32 code)=0;
};
class InstructionModule {
 Context& ctx; I32 tps;
 std::unordered_map<I32,I32> lastFrame;
 std::vector<CoreTask> coreTasks;
 bool inside(IVec2 p) const;
 I32 push(CoreTask task);
 static bool isVillager(Object& o);
 static bool isArmy(Object& o);
 static bool isPriest(Object& o);
 static bool isFishingBoat(Object& o);
 static bool isTransportBoat(Object& o);
 static bool isBuilding(Object& o);
 static bool isAnimal(Object& o);
 static bool isStaticResource(Object& o);
 static bool isShip(Object& o);
 static bool human(Object& o);
public:
 explicit InstructionModule(Context& c,I32 ticks=GAME_TPS);
 std::vector<CoreTask>& getCoreTask() noexcept;
 void tick() noexcept; // Kernel drains/clears getCoreTask after consumption.
 I32 addRelation(WeakPtr<Object> a,WeakPtr<Object> b,I32 event);
 I32 addRelation(WeakPtr<Object> a,IVec2 p,I32 event);
 I32 addRelation(WeakPtr<Object> a,IVec2 p,I32 event,I32 type);
 I32 addRelation(WeakPtr<Object> a,I32 event,I32 action);
 void filterInstruction(std::vector<Instruction>& ins,I32 coreClock);
 void resetClock();
};

