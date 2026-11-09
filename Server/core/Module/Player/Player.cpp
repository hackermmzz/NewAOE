#include "Player.h"

Player::Player(I32 civilization, I32 teamID){
    this->civilization = civilization;
    this->teamID = teamID;
    ResourceGroup = GameValue::getStartResource(civilization);
}

void Player::init(I32 playerID){
    this->playerID = playerID;
}
Army* Player::newArmy(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience){
    SafePtr<Army> a(new Army(detailPos,type,playerScience));
    a->setBelong(playerID,teamID);
    armies.push_back(a);
    return a.get();
}

Farmer* Player::newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type){
    SafePtr<Farmer> f(new Farmer(detailPos,playerScience,type));
    farmers.push_back(f);
    f->setBelong(playerID,teamID);
    return f.get();
}

Building* Player::newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 Percent){
    SafePtr<Building> b(new Building(type,blockPos,playerScience,Percent));
    buildings.push_back(b);
    b->setBelong(playerID,teamID);
    return b.get();
}

BuildingResource* Player::newBuildingResource(I32 type , IVec2 blockPos, WeakPtr<Development> playerScience,I32 Percent){
    SafePtr<BuildingResource> br(new BuildingResource(type,blockPos,playerScience,Percent));
    //只进 buildingResources 一个池子。它虽然继承自 Building，但两边各存一份的话，
    //遍历 getBuilding() 的地方会把农田当成普通建筑再算一次（占地、碰撞都会重复）
    buildingResources.push_back(br);
    br->setBelong(playerID,teamID);
    return br.get();
}

Missile* Player::newMissile(I32 type, WeakPtr<Object> attacker , WeakPtr<Object> attackee,WeakPtr<Development> playerScience,I32 attack){
    SafePtr<Missile> m(new Missile(type,attacker,attackee,playerScience,attack));
    missiles.push_back(m);
    m->setBelong(playerID,teamID);
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
        if((*it) && (*it)->getglobalNum() == globalID){
            armies.erase(it);
            ReceivedObjectLockedDevelopment.erase(globalID);
            return;
        }
    }
}

void Player::removeFarmer(I32 globalID){
    for(auto it = farmers.begin();it!=farmers.end();++it){
        if((*it) && (*it)->getglobalNum() == globalID){
            farmers.erase(it);
            ReceivedObjectLockedDevelopment.erase(globalID);
            return;
        }
    }
}

void Player::removeBuilding(I32 globalID){
    for(auto it = buildings.begin();it!=buildings.end();++it){
        if((*it) && (*it)->getglobalNum() == globalID){
            buildings.erase(it);
            ReceivedObjectLockedDevelopment.erase(globalID);
            return;
        }
    }
}

void Player::removeBuildingResource(I32 globalID){
    for(auto it = buildingResources.begin();it!=buildingResources.end();++it){
        if((*it) && (*it)->getglobalNum() == globalID){
            buildingResources.erase(it);
            ReceivedObjectLockedDevelopment.erase(globalID);
            return;
        }
    }
}

void Player::removeMissile(I32 globalID){
    for(auto it = missiles.begin();it!=missiles.end();++it){
        if((*it) && (*it)->getglobalNum() == globalID){
            missiles.erase(it);
            return;
        }
    }
}

I32 Player::getResource(I32 resourceSort){
    if(resourceSort < 0 || resourceSort >= (I32)ResourceGroup.size())return 0;
    return ResourceGroup[resourceSort];
}

array<I32,4> Player::getResource(){
    return ResourceGroup;
}

bool Player::changeResource(I32 resourceSort, I32 changeAmount){
    if(resourceSort < 0 || resourceSort >= (I32)ResourceGroup.size())return false;
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
    if(!army)return false;
    if(army->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    army->setBelong(playerID,teamID);
    armies.push_back(army);
    SafePtr<Development> sci = army->getPlayerScience().lock();//科技锁定在转换这一刻
    if(sci)ReceivedObjectLockedDevelopment[army->getglobalNum()]=SafePtr<Development>(new Development(*sci));
    return true;
}
bool Player::receiveBuilding(SafePtr<Building> building){
    if(!building)return false;
    if(building->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    building->setBelong(playerID,teamID);
    buildings.push_back(building);
    SafePtr<Development> sci = building->getPlayerScience().lock();//科技锁定在转换这一刻
    if(sci)ReceivedObjectLockedDevelopment[building->getglobalNum()]=SafePtr<Development>(new Development(*sci));
    return true;
}
bool Player::receiveFarmer(SafePtr<Farmer> farmer){
    if(!farmer)return false;
    if(farmer->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    farmer->setBelong(playerID,teamID);
    farmers.push_back(farmer);
    SafePtr<Development> sci = farmer->getPlayerScience().lock();//科技锁定在转换这一刻
    if(sci)ReceivedObjectLockedDevelopment[farmer->getglobalNum()]=SafePtr<Development>(new Development(*sci));
    return true;
}
bool Player::receiveBuildingResource(SafePtr<BuildingResource> buildingResource){
    if(!buildingResource)return false;
    if(buildingResource->getBelong().second==this->teamID)return false;//同阵营对象禁止转换
    buildingResource->setBelong(playerID,teamID);
    buildingResources.push_back(buildingResource);//只进 buildingResources，理由见 newBuildingResource
    SafePtr<Development> sci = buildingResource->getPlayerScience().lock();//科技锁定在转换这一刻
    if(sci)ReceivedObjectLockedDevelopment[buildingResource->getglobalNum()]=SafePtr<Development>(new Development(*sci));
    return true;
}

//每次完整运行完一次内核后执行：回收本帧已死亡的对象，并同步释放它们的科技快照
//注意：池子只应该通过 newXxx / removeXxx 增删，不要绕过接口直接改 getXXX() 返回的引用
void Player::tick(){
    for(auto it = armies.begin();it!=armies.end();){
        if(!(*it) || (*it)->isDie()){
            if(*it)ReceivedObjectLockedDevelopment.erase((*it)->getglobalNum());
            it = armies.erase(it);
        }else ++it;
    }
    for(auto it = farmers.begin();it!=farmers.end();){
        if(!(*it) || (*it)->isDie()){
            if(*it)ReceivedObjectLockedDevelopment.erase((*it)->getglobalNum());
            it = farmers.erase(it);
        }else ++it;
    }
    for(auto it = buildings.begin();it!=buildings.end();){
        if(!(*it) || (*it)->isDie()){
            if(*it)ReceivedObjectLockedDevelopment.erase((*it)->getglobalNum());
            it = buildings.erase(it);
        }else ++it;
    }
    for(auto it = buildingResources.begin();it!=buildingResources.end();){
        if(!(*it) || (*it)->isDie()){
            if(*it)ReceivedObjectLockedDevelopment.erase((*it)->getglobalNum());
            it = buildingResources.erase(it);
        }else ++it;
    }
    //投掷物没有血量，什么时候消失由内核（命中/超时）决定，这里只清理空槽位
    for(auto it = missiles.begin();it!=missiles.end();){
        if(!(*it))it = missiles.erase(it);
        else ++it;
    }
}