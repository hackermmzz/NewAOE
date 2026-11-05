#pragma once

#include <array>
#include "Server/core/BaseObject/Army.h"
#include "Server/core/BaseObject/Farmer.h"
#include "Server/core/BaseObject/BuildingResource.h"
#include "Server/core/BaseObject/Missile.h"
#include "Server/core/Public/var/Sort.h"
#include "Server/core/Public/config/CivilizationFeature.h"

class Player{
public:
    Player(I32 civilization, I32 teamID);
    void init(I32 playerID);
    //新建对象
    Army*               newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience , I32 playerRepresent);
    Farmer*             newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience , I32 playerRepresent,I32 type);
    Building*           newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 playerRepresent, I32 Percent);
    BuildingResource*   newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience , I32 playerRepresent ,I32 Percent);
    Missile*            newMissile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack);
    //获取对象
    vector<SafePtr<Army>>&              getArmy();
    vector<SafePtr<Farmer>>&            getFarmer();
    vector<SafePtr<Building>>&          getBuilding();
    vector<SafePtr<BuildingResource>>&  getBuildingResource();
    vector<SafePtr<Missile>>&           getMissile();
    //移除对象
    void removeArmy(I32 globalID);
    void removeFarmer(I32 globalID);
    void removeBuilding(I32 globalID);
    void removeBuildingResource(I32 globalID);
    void removeMissile(I32 globalID);
    //获取玩家资源量
    I32             getResource(I32 resourceSort);          //传入资源种类，返回该资源的数量
    array<I32,4>    getResource();              //以数组形式返回每种资源的数量，顺序为木材，粮食，石头，黄金。根据下标访问即可。
    //改变玩家资源（加资源就用正，减资源就用负，返回是否成功，不成功不会扣除相应资源）
    bool changeResource(I32 resourceSort, I32 changeAmount);        //增减一种资源的量
    bool changeResource(I32 changeWoodAmount, I32 changeFoodAmount, I32 changeStoneAmount, I32 changeGoldAmount); //增减所有资源的量(时候同时增减多种资源，不需要增减的资源填0即可)
    void tick();
private:
    //玩家身份状态信息
    I32 civilization;
    I32 teamID;
    I32 playerID;
    //玩家资源信息
    array<I32,4> ResourceGroup;                    //玩家所持有的资源量，顺序为木材，粮食，石头，黄金。
    //玩家所持有的各种对象信息
    vector<SafePtr<Army>>               armies;
    vector<SafePtr<Farmer>>             farmers;
    vector<SafePtr<Building>>           buildings;
    vector<SafePtr<BuildingResource>>   buildingResources;
    vector<SafePtr<Missile>>            missiles;
};
