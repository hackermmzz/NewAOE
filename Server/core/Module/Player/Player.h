#pragma once

#include <array>
#include <set>
#include "Server/core/BaseObject/Army.h"
#include "Server/core/BaseObject/Farmer.h"
#include "Server/core/BaseObject/BuildingResource.h"
#include "Server/core/BaseObject/Missile.h"
#include "Server/core/Public/var/Sort.h"
#include <map>

class Player{
public:
    Player(I32 civilization, I32 teamID);
    void init(I32 playerID);
    //新建对象
    Army*               newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience);
    Farmer*             newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type);
    Building*           newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience,I32 Percent);
    BuildingResource*   newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience,I32 Percent);
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
    array<I32,4>    getResource();                          //以I32[4]数组形式返回每种资源的数量，顺序为木材，粮食，石头，黄金。根据下标访问即可。
    //改变玩家资源（加资源就用正，减资源就用负，返回是否成功，不成功不会扣除相应资源）
    bool changeResource(I32 resourceSort, I32 changeAmount);        //增减一种资源的量
    bool changeResource(I32 changeWoodAmount, I32 changeFoodAmount, I32 changeStoneAmount, I32 changeGoldAmount); //增减所有资源的量(时候同时增减多种资源，不需要增减的资源填0即可)
    //接收敌对阵营的对象，并将其编入我方阵营（用于该对象被我方祭司转换的情况）
    //转化后对象的科技会改为跟随本玩家的 playerScience
    bool receiveBuilding(SafePtr<Building> building);
    bool receiveArmy(SafePtr<Army> army);
    bool receiveFarmer(SafePtr<Farmer> farmer);
    bool receiveBuildingResource(SafePtr<BuildingResource> buildingResource);
    //本玩家的科技（由第一次 newXXX 时传入的 Development 记录，只读）
    WeakPtr<Development> getPlayerScience() const;
    //内核通知：某个建筑动作已成功完成。
    //注意：内核应先让 Development 更新完科技状态，再调用本函数，否则重算到的还是旧值。
    void notifyBuildingActionFinish(WeakPtr<Object> object, I32 action);
    void tick();
private:
    //玩家身份状态信息
    I32 civilization;
    I32 teamID;
    I32 playerID;
    //玩家科技
    WeakPtr<Development> playerScience;
    //玩家资源信息
    array<I32,4> ResourceGroup;                    //玩家所持有的资源量，顺序为木材，粮食，石头，黄金。
    //玩家所持有的各种对象信息
    vector<SafePtr<Army>>               armies;
    vector<SafePtr<Farmer>>             farmers;
    vector<SafePtr<Building>>           buildings;
    vector<SafePtr<BuildingResource>>   buildingResources;
    vector<SafePtr<Missile>>            missiles;
    //已经完成过的科技/解锁类建筑动作，用来判断新建对象该给到哪一级
    std::set<I32>                       finishedActions;
    //记录本玩家的 Development：只存第一次传入的，后续重复传入不会覆盖。
    void rememberPlayerScience(WeakPtr<Development> science);
    //科技变化后，把本玩家名下需要跟随科技的对象的缓存数值重算一遍
    //（只有 Army / Farmer / Building；投掷物和农田按建成时的科技定格）。
    void refreshAllObjectScience();
protected:
    I32 ObjectCanImprove(I32 sort,I32 type);
    // __OBJECTREALLYTYPE__[code]：某个升级槽位目前已经升到的级别。
    // 初值必须是该槽位的「底层类型」，redirectObjectType 靠
    // __OBJECTREALLYTYPE__[code] <= type 判断有没有升过级。
    // 编号分配：兵种/农民 0~13，箭塔 20，城墙 21（建筑与兵种不共用槽位）。
    static const I32 __OBJECTREALLYTYPE_SIZE__ = 22;
    I32 __OBJECTREALLYTYPE__[__OBJECTREALLYTYPE_SIZE__]={
        FARMERTYPE_SMALL_SAILING,   //0  渔船链
        FARMERTYPE_SMALL_TRADE_BOAT,//1  商船链
        FARMERTYPE_LIGHT_TRANSPORT, //2  运输船链
        AT_CLUBMAN,                 //3  棍棒兵链
        AT_SHORT_SWORDSMAN,         //4  短剑兵链
        AT_IMPROVED_BOWMAN,         //5  强弓兵链
        AT_HORSE_ARCHER,            //6  弓骑兵链
        AT_CAVALRY,                 //7  骑兵链
        AT_CHARIOT,                 //8  战车链
        AT_WAR_ELEPHANT,            //9  战象链
        AT_STONE_THROWER,           //10 投石车链
        AT_BALLISTA,                //11 弩炮链
        AT_SCOUT_SHIP,              //12 战船链
        AT_HOPLITE,                 //13 方阵兵链
        0,0,0,0,0,0,                //14~19 预留
        BUILDING_WATCH_TOWER,       //20 箭塔链
        BUILDING_SMALL_WALL         //21 城墙链
    };
    I32 redirectObjectType(I32 sort,I32 type);
    void changeObjectType(I32 sort,I32 code);
};
