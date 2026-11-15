#include "Player.h"
#include "Server/core/Public/config/ObjectBaseValue.h"

namespace{
// 产人类的建筑动作：完成的结果是凭空多出一个单位，跟科技无关。
// 新单位在创建时已经拿过当时的科技，不需要为了它去重算全场。
// 其余动作（研究/升级/升时代）都会改动科技的加成，属于需要重算的范畴。
bool isUnitCreateAction(I32 action){
    switch(action){
    case BUILDING_CENTER_CREATEFARMER:
    //兵营
    case BUILDING_ARMYCAMP_CREATE_CLUBMAN:
    case BUILDING_ARMYCAMP_CREATE_SHORTSWORD:
    case BUILDING_ARMYCAMP_CREATE_SLINGER:
    //靶场
    case BUILDING_RANGE_CREATE_BOWMAN:
    case BUILDING_RANGE_CREATE_COMPOSITE_BOWMAN:
    case BUILDING_RANGE_CREATE_CHARIOT_ARCHER:
    case BUILDING_RANGE_CREATE_HORSE_ARCHER:
    case BUILDING_RANGE_CREATE_ELEPHANT_ARCHER:
    //马厩
    case BUILDING_STABLE_CREATE_SCOUT:
    case BUILDING_STABLE_CREATE_CAVALRY:
    case BUILDING_STABLE_CREATE_CHARIOT:
    case BUILDING_STABLE_CREATE_WAR_ELEPHANT:
    case BUILDING_STABLE_CREATE_CAMEL:
    //船坞
    case BUILDING_DOCK_CREATE_SAILING:
    case BUILDING_DOCK_CREATE_TRADEBOAT:
    case BUILDING_DOCK_CREATE_TRANSPORT:
    case BUILDING_DOCK_CREATE_SCOUTSHIP:
    case BUILDING_DOCK_CREATE_CATAPULT_TRIREME:
    case BUILDING_DOCK_CREATE_FIREGALLEY:
    //攻城工坊
    case BUILDING_SIEGE_CREATE_STONE_THROWER:
    case BUILDING_SIEGE_CREATE_BALLISTA:
    //学院 / 神殿
    case BUILDING_COLLAGE_CREATE_HOPLITE:
    case BUILDING_TEMPLE_CREATE_PRIEST:
        return true;
    default:
        return false;
    }
}

// 一条升级规则：某个建筑动作完成后，把 fromType 的对象整体变成 toType。
// fromType 传 -1 表示这是一条链的起点（只是解锁，场上没有前一级对象）。
// upgradeExisting 区分「已有对象跟着升级」（兵种、箭塔）和「只影响以后新建的」
// （城墙：研究完只解锁更高级城墙，已经砌好的城墙不变）。
struct UpgradeRule{
    I32  action;            //触发升级的建筑动作
    I32  sort;              //对象大类
    I32  fromType;          //升级前的具体类型，-1 表示链首
    I32  toType;            //升级后的具体类型
    bool upgradeExisting;   //场上已有的前一级对象是否一起升级
};

// 升级链取自 Public/var 下各枚举与策划给的升级顺序。
constexpr UpgradeRule kUpgradeRules[] = {
    //---- 兵营 ----
    //棍棒兵 → 刀斧兵
    {BUILDING_ARMYCAMP_UPGRADE_CLUBMAN,      SORT_ARMY, AT_CLUBMAN,          AT_AXEMAN,          true},
    //短剑兵 → 阔剑兵 → 长剑兵 → 军团步兵
    {BUILDING_ARMYCAMP_UPGRADE_BROADSWORD,   SORT_ARMY, AT_SHORT_SWORDSMAN,  AT_BROAD_SWORDSMAN, true},
    {BUILDING_ARMYCAMP_UPGRADE_LONGSWORD,    SORT_ARMY, AT_BROAD_SWORDSMAN,  AT_LONG_SWORDSMAN,  true},
    {BUILDING_ARMYCAMP_UPGRADE_LEGION,       SORT_ARMY, AT_LONG_SWORDSMAN,   AT_LEGION,          true},
    //---- 靶场 ----
    //强弓兵 → 复合弓兵；弓骑兵 → 重装弓骑兵
    {BUILDING_RANGE_UPGRADE_COMPOSITE_BOW,   SORT_ARMY, AT_IMPROVED_BOWMAN,  AT_COMPOSITE_BOWMAN,true},
    {BUILDING_RANGE_UPGRADE_HORSE_ARCHER,    SORT_ARMY, AT_HORSE_ARCHER,     AT_HEAVY_HORSE_ARCHER,true},
    //---- 马厩 ----
    //骑兵 → 重骑兵 → 铁甲骑兵；战车 → 刀轮战车；战象 → 装甲战象
    {BUILDING_STABLE_UPGRADE_CAVALRY,        SORT_ARMY, AT_CAVALRY,          AT_HEAVY_CAVALRY,   true},
    {BUILDING_STABLE_UPGRADE_CATAPHRACT,     SORT_ARMY, AT_HEAVY_CAVALRY,    AT_CATAPHRACT,      true},
    {BUILDING_STABLE_UPGRADE_CHARIOT,        SORT_ARMY, AT_CHARIOT,          AT_SCYTHECHARIOT,   true},
    {BUILDING_STABLE_UPGRADE_ELEPHANT,       SORT_ARMY, AT_WAR_ELEPHANT,     AT_ARMORED_ELEPHANT,true},
    //---- 攻城武器厂 ----
    //投石车 → 中型投石车 → 重型投石车；弩炮 → 重型弩炮
    {BUILDING_SIEGE_UPGRADE_CATAPULT,        SORT_ARMY, AT_STONE_THROWER,    AT_CATAPULT,        true},
    {BUILDING_SIEGE_UPGRADE_HEAVYCATAPULT,   SORT_ARMY, AT_CATAPULT,         AT_HEAVY_CATAPULT,  true},
    {BUILDING_SIEGE_UPGRADE_HELEPOLIS,       SORT_ARMY, AT_BALLISTA,         AT_HELEPOLIS,       true},
    //---- 船坞 ----
    //侦察船 → 桨帆船 → 三列桨帆船
    {BUILDING_DOCK_UPGRADE_WAR_GALLEY,       SORT_ARMY, AT_SCOUT_SHIP,       AT_WAR_GALLEY,      true},
    {BUILDING_DOCK_UPGRADE_TRIREME,          SORT_ARMY, AT_WAR_GALLEY,       AT_TRIREME,         true},
    //渔船 → 大型渔船；商船 → 大型商船；运输船 → 重型运输船
    {BUILDING_DOCK_UPGRADE_SAILING,          SORT_FARMER, FARMERTYPE_SMALL_SAILING,     FARMERTYPE_BIG_SAILING,     true},
    {BUILDING_DOCK_UPGRADE_TRADEBOAT,        SORT_FARMER, FARMERTYPE_SMALL_TRADE_BOAT,  FARMERTYPE_BIG_TRADE_BOAT,  true},
    {BUILDING_DOCK_UPGRADE_TRANSPORT,        SORT_FARMER, FARMERTYPE_LIGHT_TRANSPORT,  FARMERTYPE_HEAVY_TRANSPORT, true},
    //---- 学院 ----
    //方阵兵 → 十夫长 → 百夫长
    {BUILDING_COLLAGE_UPGRADE_PHALANX,       SORT_ARMY, AT_HOPLITE,          AT_PHALANX,         true},
    {BUILDING_COLLAGE_UPGRADE_CENTURION,     SORT_ARMY, AT_PHALANX,          AT_CENTURION,       true},
    //---- 城墙：只解锁更高级的城墙，已经砌好的不跟着变 ----
    {BUILDING_GRANARY_WALL,                  SORT_BUILDING, -1,                  BUILDING_SMALL_WALL,    false},
    {BUILDING_GRANARY_WALL_UPDATE_1,         SORT_BUILDING, BUILDING_SMALL_WALL, BUILDING_MEDIUM_WALL,   false},
    {BUILDING_GRANARY_WALL_UPDATE_2,         SORT_BUILDING, BUILDING_MEDIUM_WALL,BUILDING_FORTIFIED_WALL,false},
    //---- 箭塔：瞭望 → 警戒 → 防御 → 防御炮塔 ----
    {BUILDING_GRANARY_ARROWTOWER,            SORT_BUILDING, -1,                  BUILDING_WATCH_TOWER,   false},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_1,  SORT_BUILDING, BUILDING_WATCH_TOWER,  BUILDING_SENTRY_TOWER,  true},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_2,  SORT_BUILDING, BUILDING_SENTRY_TOWER, BUILDING_GUARD_TOWER,   true},
    {BUILDING_GRANARY_ARROWTOWER_UPGRADE_3,  SORT_BUILDING, BUILDING_GUARD_TOWER,  BUILDING_BALLISTA_TOWER,true},
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
    if(rule == nullptr || rule->fromType < 0 || !rule->upgradeExisting){
        //没配升级链 / 是链首（纯解锁）/ 只影响以后新建的（城墙），场上没有要改的对象
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