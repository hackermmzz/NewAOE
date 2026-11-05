#pragma once

#include "Server/core/BaseObject/Army.h"
#include "Server/core/BaseObject/Farmer.h"
#include "Server/core/BaseObject/BuildingResource.h"
#include "Server/core/BaseObject/Missile.h"

class Player{
public:
    Player(I32 civilization, I32 teamID);
    void init(I32 playerID);
    //新建对象
    Army* newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience , I32 playerRepresent);
    Farmer* newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience , I32 playerRepresent,I32 type);
    Building* newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 playerRepresent, I32 Percent);
    BuildingResource* newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience , I32 playerRepresent ,I32 Percent);
    Missile* newMissile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack);
    //获取对象
    vector<SafePtr<Army>>& getArmy();
    vector<SafePtr<Farmer>>& getFarmer();
    vector<SafePtr<Building>>& getBuilding();
    vector<SafePtr<BuildingResource>>& getBuildingResource();
    vector<SafePtr<Missile>>& getMissile();
    //移除对象
    void removeArmy(I32 globalID);
    void removeFarmer(I32 globalID);
    void removeBuilding(I32 globalID);
    void removeBuildingResource(I32 globalID);
    void removeMissile(I32 globalID);
    //改变玩家资源（加资源就用正，减资源就用负）
    bool changeWood (I32 amount);
    bool changeFood (I32 amount);
    bool changeGold (I32 amount);
    bool changeStone(I32 amount);

    void tick();
private:
    //玩家身份状态信息
    I32 civilization;
    I32 teamID;
    I32 playerID;
    //玩家资源信息
    I32 wood;
    I32 food;
    I32 gold;
    I32 stone;
    //玩家所持有的各种对象信息
    vector<SafePtr<Army>> armies;
    vector<SafePtr<Farmer>> farmers;
    vector<SafePtr<Building>> buildings;
    vector<SafePtr<BuildingResource>> buildingResources;
    vector<SafePtr<Missile>> missiles;
};
