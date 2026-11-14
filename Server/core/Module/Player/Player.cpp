#include "Player.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

Player::Player(I32 civilization, I32 teamID){
    this->civilization = civilization;
    this->teamID = teamID;
    ResourceGroup = ObjectBaseValue::getStartResource(civilization);
}

void Player::init(I32 playerID){
    this->playerID = playerID;
}
Army* Player::newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience){
    rememberPlayerScience(playerScience);
    SafePtr<Army> a(new Army(detailPos,type,playerScience));
    a->setBelong(playerID,teamID);
    armies.push_back(a);
    return a.get();
}

Farmer* Player::newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type){
    rememberPlayerScience(playerScience);
    SafePtr<Farmer> f(new Farmer(detailPos,playerScience,type));
    farmers.push_back(f);
    f->setBelong(playerID,teamID);
    return f.get();
}

Building* Player::newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 Percent){
    rememberPlayerScience(playerScience);
    SafePtr<Building> b(new Building(type,blockPos,playerScience,Percent));
    buildings.push_back(b);
    b->setBelong(playerID,teamID);
    return b.get();
}

BuildingResource* Player::newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience,I32 Percent){
    rememberPlayerScience(playerScience);
    SafePtr<BuildingResource> br(new BuildingResource(type,blockPos,playerScience,Percent));
    // 资源型建筑（农田等）只登记在 buildingResources 里，不再同时塞进 buildings，
    // 否则同一个对象会被两个容器各持有一份 SafePtr，删除时相互脱节。
    buildingResources.push_back(br);
    br->setBelong(playerID,teamID);
    return br.get();
}

Missile* Player::newMissile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack){
    rememberPlayerScience(playerScience);
    SafePtr<Missile> m(new Missile(type,attacker,attackee,playerScience,attack));
    missiles.push_back(m);
    m->setBelong(playerID,teamID);
    return m.get();
}

// 只认第一次拿到的 Development：玩家自己的科技由外部创建后传入，
// 后续 newXXX 传进来的应当还是同一个，重复写入没有意义。
void Player::rememberPlayerScience(WeakPtr<Development> science){
    if(playerScience.expired() && !science.expired()){
        playerScience = science;
    }
}

WeakPtr<Development> Player::getPlayerScience() const{
    return playerScience;
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
        if((*it)->getglobalNum() == globalID){
            armies.erase(it);
            return;
        }
    }
}

void Player::removeFarmer(I32 globalID){
    for(auto it = farmers.begin();it!=farmers.end();++it){
        if((*it)->getglobalNum() == globalID){
            farmers.erase(it);
            return;
        }
    }
}

void Player::removeBuilding(I32 globalID){
    for(auto it = buildings.begin();it!=buildings.end();++it){
        if((*it)->getglobalNum() == globalID){
            buildings.erase(it);
            return;
        }
    }
}

void Player::removeBuildingResource(I32 globalID){
    for(auto it = buildingResources.begin();it!=buildingResources.end();++it){
        if((*it)->getglobalNum() == globalID){
            buildingResources.erase(it);
            return;
        }
    }
}

void Player::removeMissile(I32 globalID){
    for(auto it = missiles.begin();it!=missiles.end();++it){
        if((*it)->getglobalNum() == globalID){
            missiles.erase(it);
            return;
        }
    }
}

I32 Player::getResource(I32 resourceSort){
    return ResourceGroup[resourceSort];
}

array<I32,4> Player::getResource(){
    return ResourceGroup;
}

bool Player::changeResource(I32 resourceSort, I32 changeAmount){
    if(ResourceGroup[resourceSort] + changeAmount < 0){
        return false;
    }
    ResourceGroup[resourceSort] += changeAmount;
    return true;
}

bool Player::changeResource(I32 changeWoodAmount, I32 changeFoodAmount, I32 changeStoneAmount, I32 changeGoldAmount){
    if(ResourceGroup[RESOURCESORT_WOOD] + changeWoodAmount < 0 || ResourceGroup[RESOURCESORT_FOOD] + changeFoodAmount < 0 || ResourceGroup[RESOURCESORT_STONE] + changeStoneAmount < 0 || ResourceGroup[RESOURCESORT_GOLD] + changeGoldAmount < 0){
        return false;
    }
    ResourceGroup[RESOURCESORT_WOOD]  += changeWoodAmount;
    ResourceGroup[RESOURCESORT_FOOD]  += changeFoodAmount;
    ResourceGroup[RESOURCESORT_STONE] += changeStoneAmount;
    ResourceGroup[RESOURCESORT_GOLD]  += changeGoldAmount;
    return true;
}

bool Player::receiveArmy(SafePtr<Army> army){
    if(army->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    army->setBelong(playerID,teamID);
    // 转化后的单位科技跟随本玩家，不再拷贝一份锁定在原阵营转换时刻的科技。
    army->setPlayerScience(playerScience);
    armies.push_back(army);
    return true;
}
bool Player::receiveBuilding(SafePtr<Building> building){
    if(building->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    building->setBelong(playerID,teamID);
    building->setPlayerScience(playerScience);
    buildings.push_back(building);
    return true;
}
bool Player::receiveFarmer(SafePtr<Farmer> farmer){
    if(farmer->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    farmer->setBelong(playerID,teamID);
    farmer->setPlayerScience(playerScience);
    farmers.push_back(farmer);
    return true;
}
bool Player::receiveBuildingResource(SafePtr<BuildingResource> buildingResource){
    if(buildingResource == nullptr)return false;
    if(buildingResource->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    // 不能复用 receiveBuilding：资源型建筑不进入 buildings 容器。
    buildingResource->setBelong(playerID,teamID);
    buildingResource->setPlayerScience(playerScience);
    buildingResources.push_back(buildingResource);
    return true;
}