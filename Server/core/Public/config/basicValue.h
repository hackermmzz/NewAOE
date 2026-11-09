#pragma once


#include <vector>
#include <array>
#include <utility>
#include <map>
#include "Public/type/type.h"
#include "Server/conf.h"
#include "Server/core/type.h"
#include "Server/core/Public/var/Civilization.h"
#include "Server/core/Public/var/Sort.h"

struct Building_BasicBlood{
    I32 House;
    I32 Barracks;
    I32 Stable;
    I32 ArcheryRange;
    I32 Market;
    I32 Blacksmith;
    I32 University;
    I32 Castle;
    I32 Wall;
    I32 Gate;
    I32 Tower;
    I32 Wonder;
    I32 Dock;
    I32 Mill;
    //未完成 缺数值
};

struct Farmer_BasicBlood{
    I32 Villager;
    I32 Scout;
    I32 Monk;
    //未完成
};

struct Animal_BasicBlood{
    I32 Gazelle;
    I32 Elephant;
    I32 Lion;
    I32 Alligator;
    //未完成
};

struct Army_BasicBlood{
    I32 Swordsman;
    I32 Spearman;
    I32 Archer;
    I32 Cavalry;
    I32 Knight;
    I32 Crossbowman;
    I32 Catapult;
    I32 Trebuchet;
    I32 Cannon;
    I32 GunpowderInfantry;
    I32 Musketman;
    I32 Hussar;
    I32 CavalryArcher;
    I32 HeavyCavalry;
    I32 HeavyInfantry;
    //未完成
};

struct All_BasicBlood{
    Building_BasicBlood Building;
    Farmer_BasicBlood Farmer;
    Animal_BasicBlood Animal;
    Army_BasicBlood Army;
};

struct __GameBasicValue{
    //开局初始资源
    static constexpr I32 BasicStartResource[4]={200,200,150,0};
    //各单位初始血量
    /*static constexpr Building_BasicBlood BuildingBasicBlood = {};
    static constexpr Farmer_BasicBlood FarmerBasicBlood = {};
    static constexpr Animal_BasicBlood AnimalBasicBlood = {};
    static constexpr Army_BasicBlood ArmyBasicBlood = {};   原始形式*/
    static constexpr const All_BasicBlood BasicBlood = {};

    static constexpr const Building_BasicBlood& Building() { return BasicBlood.Building; }
    static constexpr const Farmer_BasicBlood&   Farmer()   { return BasicBlood.Farmer; }
    static constexpr const Animal_BasicBlood&   Animal()   { return BasicBlood.Animal; }
    static constexpr const Army_BasicBlood&     Army()     { return BasicBlood.Army; }
    //static constexpr const I32* BasicBlood[7]={BuildingBasicBlood,nullptr,FarmerBasicBlood,AnimalBasicBlood,nullptr,BuildingBasicBlood,ArmyBasicBlood};
    
};
