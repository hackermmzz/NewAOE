#include "Player.h"
#include "Server/core/Public/config/ObejctBaseValue.h"

namespace{
// 产人类的建筑动作：完成的结果是凭空多出一个单位，跟科技无关。
// 新单位在创建时已经拿过当时的科技，不需要为了它去重算全场。
// 其余动作（研究/升级/升时代）都会改动科技的加成，属于需要重算的范畴。
bool isUnitCreateAction(I32 action){
    switch(action){
    case BUILDING_CENTER_CREATEFARMER:
    case BUILDING_ARMYCAMP_CREATE_CLUBMAN:
    case BUILDING_ARMYCAMP_CREATE_SLINGER:
    case BUILDING_ARMYCAMP_CREATE_BROADSWORD:
    case BUILDING_RANGE_CREATE_BOWMAN:
    case BUILDING_RANGE_CREATE_CHARIOT_ARCHER:
    case BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN:
    case BUILDING_STABLE_CREATE_SCOUT:
    case BUILDING_STABLE_CREATE_CHARIOT:
    case BUILDING_STABLE_CREATE_CAVALRY:
    case BUILDING_DOCK_CREATE_SAILING:
    case BUILDING_DOCK_CREATE_WOOD_BOAT:
    case BUILDING_DOCK_CREATE_SHIP:
    case BUILDING_SIEGE_CREATE_STONE_THROWER:
    case BUILDING_COLLAGE_CREATE_HOPLITE:
        return true;
    default:
        return false;
    }
}

// 一条升级规则：某个建筑动作完成后，把 fromType 的对象整体变成 toType。
// fromType 传 -1 表示这个动作只是「解锁」，场上没有需要升级的旧对象（比如研发城墙/箭塔）。
struct UpgradeRule{
    I32 action;     //触发升级的建筑动作
    I32 sort;       //对象大类
    I32 fromType;   //升级前的具体类型
    I32 toType;     //升级后的具体类型
};

// 升级链取自 Public/var 下各枚举里的分组：同一组内相邻两项就是上下级。
// 目前 BuildingAction 里能触发的升级项就这些，枚举补齐后往这里加即可。
constexpr UpgradeRule kUpgradeRules[] = {
    //兵营：棍棒兵 → 斧兵
    {BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,     SORT_ARMY, AT_CLUBMAN,          AT_AXEMAN},
    //兵营：短剑兵 → 阔剑兵
    {BUILDING_ARMYCAMP_UPGRADE_BROADSWORD,  SORT_ARMY, AT_SHORT_SWORDSMAN,  AT_BROAD_SWORDSMAN},
    //靶场：强弓兵 → 复合弓兵
    {BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,  SORT_ARMY, AT_IMPROVED_BOWMAN,  AT_COMPOSITE_BOWMAN},
    //城墙：解锁初级 → 中级 → 高级
    {BUILDING_GRANARY_WALL,                 SORT_BUILDING, -1,                     BUILDING_SMALL_WALL},
    {BUILDING_GRANARY_WALL_UPDATE_1,        SORT_BUILDING, BUILDING_SMALL_WALL,    BUILDING_MEDIUM_WALL},
    {BUILDING_GRANARY_WALL_UPDATE_2,        SORT_BUILDING, BUILDING_MEDIUM_WALL,   BUILDING_FORTIFIED_WALL},
    //箭塔：解锁瞭望塔 → 警戒 → 防御 → 防御炮塔
    {BUILDING_GRANARY_ARROWTOWER,           SORT_BUILDING, -1,                     BUILDING_WATCH_TOWER},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_1, SORT_BUILDING, BUILDING_WATCH_TOWER,   BUILDING_SENTRY_TOWER},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_2, SORT_BUILDING, BUILDING_SENTRY_TOWER,  BUILDING_GUARD_TOWER},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_3, SORT_BUILDING, BUILDING_GUARD_TOWER,   BUILDING_BALLISTA_TOWER},
};

//按动作查升级规则，没有就返回 nullptr
const UpgradeRule* findUpgradeRule(I32 action){
    for(const UpgradeRule& rule : kUpgradeRules){
        if(rule.action == action){
            return &rule;
        }
    }
    return nullptr;
}

//查「某个大类下，从 fromType 往上一级」的规则
const UpgradeRule* findUpgradeRuleByFrom(I32 sort,I32 fromType){
    for(const UpgradeRule& rule : kUpgradeRules){
        if(rule.sort == sort && rule.fromType == fromType){
            return &rule;
        }
    }
    return nullptr;
}

// 把已经被 destroy 掉（SafePtr 取出来是空）的对象从容器里摘干净，
// 避免容器里长期挂着空槽位。
template<class T>
void purgeDestroyed(vector<SafePtr<T>>& container){
    for(auto it = container.begin();it != container.end();){
        if(it->IsNull()){
            it = container.erase(it);
        }else{
            ++it;
        }
    }
}
}

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
    //内核传的可能是底层兵种，实际按当前科技给到已解锁的最高级别
    type = resolveCreateType(SORT_ARMY,type);
    SafePtr<Army> a(new Army(detailPos,type,playerScience));
    a->setBelong(playerID,teamID);
    armies.push_back(a);
    return a.get();
}

Farmer* Player::newFarmer(IVec2 detailPos , WeakPtr<Development> playerScience,I32 type){
    rememberPlayerScience(playerScience);
    type = resolveCreateType(SORT_FARMER,type);
    SafePtr<Farmer> f(new Farmer(detailPos,playerScience,type));
    farmers.push_back(f);
    f->setBelong(playerID,teamID);
    return f.get();
}

Building* Player::newBuilding(I32 type,IVec2 blockPos, WeakPtr<Development> playerScience, I32 Percent){
    rememberPlayerScience(playerScience);
    type = resolveCreateType(SORT_BUILDING,type);
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
    if(resourceSort < 0 || resourceSort >= static_cast<I32>(ResourceGroup.size())){
        return 0;
    }
    return ResourceGroup[resourceSort];
}

array<I32,4> Player::getResource(){
    return ResourceGroup;
}

bool Player::changeResource(I32 resourceSort, I32 changeAmount){
    //资源种类来自 ResourceSort（木材/食物/石头/黄金），越界一律拒绝，
    //避免把 ProductSort 之类的枚举误传进来直接越界写。
    if(resourceSort < 0 || resourceSort >= static_cast<I32>(ResourceGroup.size())){
        return false;
    }
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

void Player::notifyBuildingActionFinish(WeakPtr<Object> object, I32 action){
    //触发通知的建筑可能在这一帧里已经被拆掉了，对象没了就不用管。
    if(object.expired()){
        return;
    }
    //造人：结果是新单位，与科技无关，新单位创建时已经带上当前科技。
    if(isUnitCreateAction(action)){
        return;
    }
    //记下这个科技/解锁动作：之后新建对象要按它决定实际等级
    finishedActions.insert(action);
    //场上已经存在的低级别对象直接升到新级别（棍棒兵→斧兵这种）
    upgradeObjectsForAction(action);
    //科技变了，血量/攻防/射程等缓存值重算。
    //注意内核要先让 Development 更新完科技状态，再调这里。
    refreshAllObjectScience();
}

void Player::upgradeObjectsForAction(I32 action){
    const UpgradeRule* rule = findUpgradeRule(action);
    if(rule == nullptr || rule->fromType < 0){
        //没配升级链，或者只是单纯解锁（研发城墙/箭塔），场上没有要升级的对象
        return;
    }
    switch(rule->sort){
    case SORT_ARMY:{
        for(auto& army : armies){
            if(!army.IsNull() && army->getType() == rule->fromType){
                army->setType(rule->toType);
            }
        }
        break;
    }
    case SORT_FARMER:{
        for(auto& farmer : farmers){
            if(!farmer.IsNull() && farmer->getType() == rule->fromType){
                farmer->setType(rule->toType);
            }
        }
        break;
    }
    case SORT_BUILDING:{
        for(auto& building : buildings){
            if(!building.IsNull() && building->getType() == rule->fromType){
                building->setType(rule->toType);
            }
        }
        break;
    }
    default:
        break;
    }
}

I32 Player::resolveCreateType(I32 sort,I32 type) const{
    I32 current = type;
    //沿着升级链一路往上找，直到下一级的解锁动作还没完成为止。
    //已经是高级别的对象（比如转化来的重骑兵）不会往回退，因为链上找不到比它更高的就先停。
    for(;;){
        const UpgradeRule* next = findUpgradeRuleByFrom(sort,current);
        if(next == nullptr){
            break;
        }
        if(finishedActions.find(next->action) == finishedActions.end()){
            break;
        }
        current = next->toType;
    }
    return current;
}

void Player::refreshAllObjectScience(){
    //投掷物和资源型建筑按建成时的科技定格，不随之后的研究变化，所以不刷新。
    for(auto& army : armies){
        if(!army.IsNull())army->refreshByScience();
    }
    for(auto& farmer : farmers){
        if(!farmer.IsNull())farmer->refreshByScience();
    }
    for(auto& building : buildings){
        if(!building.IsNull())building->refreshByScience();
    }
}

void Player::tick(){
    //内核跑完一帧后调用。对象的增删由内核通过 newXXX/removeXXX 完成，
    //这里只做一件兜底的事：把已经被 destroy 掉的对象从容器里摘掉。
    purgeDestroyed(armies);
    purgeDestroyed(farmers);
    purgeDestroyed(buildings);
    purgeDestroyed(buildingResources);
    purgeDestroyed(missiles);
}