#include "Player.h"

Player::Player(I32 civilization, I32 teamID){
    this->civilization = civilization;
    this->teamID = teamID;
}

Army* Player::newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience , I32 playerRepresent){
    SafePtr<Army> a(new Army(detailPos,type,playerScience,playerRepresent));
    armies.emplace_back(a);
    return a.get();
}

Farmer* Player::newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience , I32 playerRepresent,I32 type){
    SafePtr<Farmer> f(new Farmer(detailPos,playerScience,playerRepresent,type));
    farmers.emplace_back(f);
    return f.get();
}

Building* Player::newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 playerRepresent, I32 Percent){
    SafePtr<Building> b(new Building(type,blockPos,playerScience,playerRepresent));
    buildings.emplace_back(b);
    return b.get();
}

BuildingResource* Player::newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience , I32 playerRepresent ,I32 Percent){
    SafePtr<BuildingResource> br(new BuildingResource(type,blockPos,playerScience,playerRepresent,Percent));
    buildingResources.emplace_back(br);
    return br.get();
}

Missile* Player::newMissile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack){
    SafePtr<Missile> m(new Missile(type,attacker,attackee,playerScience,attack));
    missiles.emplace_back(m);
    return m.get();
}

vector<SafePtr<Army>>& Player::getArmy(){
    return armies;
}

vector<SafePtr<Farmer>>& Player::getFarmer(){
    return farmers;
}

vector<SafePtr<Building>>& Player::getBuilding(){
    return buildings;
}

vector<SafePtr<BuildingResource>>& Player::getBuildingResource(){
    return buildingResources;
}

vector<SafePtr<Missile>>& Player::getMissile(){
    return missiles;
}

void Player::removeArmy(I32 globalID){
    for(auto it = armies.begin();it!=armies.end();++it){
        if((*it)->getPlayerRepresent() == globalID){
            armies.erase(it);
            return;
        }
    }
}

void Player::removeFarmer(I32 globalID){
    for(auto it = farmers.begin();it!=farmers.end();++it){
        if((*it)->getPlayerRepresent() == globalID){
            farmers.erase(it);
            return;
        }
    }
}

void Player::removeBuilding(I32 globalID){
    for(auto it = buildings.begin();it!=buildings.end();++it){
        if((*it)->getPlayerRepresent() == globalID){
            buildings.erase(it);
            return;
        }
    }
}

void Player::removeBuildingResource(I32 globalID){
    for(auto it = buildingResources.begin();it!=buildingResources.end();++it){
        if((*it)->getPlayerRepresent() == globalID){
            buildingResources.erase(it);
            return;
        }
    }
}

void Player::removeMissile(I32 globalID){
    for(auto it = missiles.begin();it!=missiles.end();++it){
        if((*it)->getPlayerRepresent() == globalID){
            missiles.erase(it);
            return;
        }
    }
}

bool Player::changeWood (I32 amount){
    if(wood + amount < 0) return false;
    wood += amount;
    return true;
}
bool Player::changeFood (I32 amount){
    if(food + amount < 0) return false;
    food += amount;
    return true;
}
bool Player::changeGold (I32 amount){
    if(gold + amount < 0) return false;
    gold += amount;
    return true;
}
bool Player::changeStone(I32 amount){
    if(stone + amount < 0) return false;
    stone += amount;
    return true;
}

