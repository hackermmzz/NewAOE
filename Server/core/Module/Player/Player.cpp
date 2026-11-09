#include "Player.h"

#include <limits>
#include <set>
#include <utility>

/*
Player 是"玩家资产表 + 规则账本"：
  - 持有各类对象的强引用（SafePtr），对象的生死由这里决定；
  - 持有四种资源的账本；
  - 不驱动对象行为。对象的位置、血量、目标由内核/指令模块驱动，
    Player 只在帧末做一次账本收尾（tick）。
*/

namespace {

// 按对象编号从池子里摘除。找不到返回 false。
template <typename T>
bool eraseByID(std::vector<SafePtr<T>>& pool, I32 globalID)
{
    for (auto it = pool.begin(); it != pool.end(); ++it) {
        if ((*it) != nullptr && (*it)->getglobalNum() == globalID) {
            pool.erase(it);
            return true;
        }
    }
    return false;
}

// 回收已失效 / 已死亡的对象，并清掉它们的科技快照。
// 注意：SafePtr 失效后不会自己从 vector 里消失，必须由这里显式擦除。
template <typename T>
void collectDead(std::vector<SafePtr<T>>& pool,
                 std::map<I32, SafePtr<Development>>& locked)
{
    for (auto it = pool.begin(); it != pool.end();) {
        if (!(*it) || (*it)->isDie()) {
            if (*it) {
                locked.erase((*it)->getglobalNum());
            }
            it = pool.erase(it);
        } else {
            ++it;
        }
    }
}

// 复制一份科技表。对象被转化后，科技锁定在转化那一刻，不再跟随原玩家。
SafePtr<Development> snapshotScience(WeakPtr<Development> science)
{
    SafePtr<Development> source = science.lock();
    if (!source) {
        return SafePtr<Development>();
    }
    return SafePtr<Development>(new Development(*source));
}

}  // namespace

Player::Player(I32 civilization, I32 teamID)
    : civilization(civilization), teamID(teamID), playerID(-1)
{
    ResourceGroup = GameValue::getStartResource(civilization);
}

void Player::init(I32 playerID)
{
    this->playerID = playerID;

    // 兜底：如果对象在 init 之前就建好了，这里补一次归属
    for (auto& a : armies)            { if (a) a->setBelong(playerID, teamID); }
    for (auto& f : farmers)           { if (f) f->setBelong(playerID, teamID); }
    for (auto& b : buildings)         { if (b) b->setBelong(playerID, teamID); }
    for (auto& r : buildingResources) { if (r) r->setBelong(playerID, teamID); }
    for (auto& m : missiles)          { if (m) m->setBelong(playerID, teamID); }
}

/* ------------------------------------------------------------ 新建对象 */

Army* Player::newArmy(IVec2 detailPos, I32 type, WeakPtr<Development> playerScience)
{
    SafePtr<Army> army(new Army(detailPos, type, playerScience));
    army->setBelong(playerID, teamID);
    armies.push_back(army);
    return army.get();
}

Farmer* Player::newFarmer(IVec2 detailPos, WeakPtr<Development> playerScience, I32 type)
{
    SafePtr<Farmer> farmer(new Farmer(detailPos, playerScience, type));
    farmer->setBelong(playerID, teamID);
    farmers.push_back(farmer);
    return farmer.get();
}

Building* Player::newBuilding(I32 type, IVec2 blockPos,
                              WeakPtr<Development> playerScience, I32 Percent)
{
    SafePtr<Building> building(new Building(type, blockPos, playerScience, Percent));
    building->setBelong(playerID, teamID);
    buildings.push_back(building);
    return building.get();
}

BuildingResource* Player::newBuildingResource(I32 type, IVec2 blockPos,
                                              WeakPtr<Development> playerScience, I32 Percent)
{
    SafePtr<BuildingResource> resource(
        new BuildingResource(type, blockPos, playerScience, Percent));
    resource->setBelong(playerID, teamID);

    // 只进 buildingResources 一个池子。它虽然继承自 Building，但如果在两个池子里
    // 各存一份，任何"遍历所有建筑"的逻辑都会把它算两次（占地、人口、碰撞都会重复），
    // 删除和转化时也要成对处理。内核需要建筑列表时，应同时遍历 getBuilding() 与
    // getBuildingResource()。
    buildingResources.push_back(resource);
    return resource.get();
}

Missile* Player::newMissile(I32 type, WeakPtr<Object> attacker, WeakPtr<Object> attackee,
                            WeakPtr<Development> playerScience, I32 attack)
{
    SafePtr<Missile> missile(new Missile(type, attacker, attackee, playerScience, attack));
    missile->setBelong(playerID, teamID);
    missiles.push_back(missile);
    return missile.get();
}

/* ------------------------------------------------------------ 获取对象 */

vector<SafePtr<Army>>& Player::getArmy()
{
    return armies;
}

vector<SafePtr<Farmer>>& Player::getFarmer()
{
    return farmers;
}

vector<SafePtr<Building>>& Player::getBuilding()
{
    return buildings;
}

vector<SafePtr<BuildingResource>>& Player::getBuildingResource()
{
    return buildingResources;
}

vector<SafePtr<Missile>>& Player::getMissile()
{
    return missiles;
}

/* ------------------------------------------------------------ 移除对象 */

void Player::removeArmy(I32 globalID)
{
    if (eraseByID(armies, globalID)) {
        ReceivedObjectLockedDevelopment.erase(globalID);
    }
}

void Player::removeFarmer(I32 globalID)
{
    if (eraseByID(farmers, globalID)) {
        ReceivedObjectLockedDevelopment.erase(globalID);
    }
}

void Player::removeBuilding(I32 globalID)
{
    if (eraseByID(buildings, globalID)) {
        ReceivedObjectLockedDevelopment.erase(globalID);
    }
}

void Player::removeBuildingResource(I32 globalID)
{
    if (eraseByID(buildingResources, globalID)) {
        ReceivedObjectLockedDevelopment.erase(globalID);
    }
}

void Player::removeMissile(I32 globalID)
{
    eraseByID(missiles, globalID);
}

/* ------------------------------------------------------------ 资源账本 */

I32 Player::getResource(I32 resourceSort)
{
    if (resourceSort < 0 || resourceSort >= static_cast<I32>(ResourceGroup.size())) {
        return 0;
    }
    return ResourceGroup[resourceSort];
}

array<I32, 4> Player::getResource()
{
    return ResourceGroup;
}

bool Player::changeResource(I32 resourceSort, I32 changeAmount)
{
    if (resourceSort < 0 || resourceSort >= static_cast<I32>(ResourceGroup.size())) {
        return false;
    }
    // 用 I64 判断，避免 INT32_MIN 取负时溢出；不成功时账本保持原样
    const I64 next = static_cast<I64>(ResourceGroup[resourceSort]) + changeAmount;
    if (next < 0 || next > static_cast<I64>(std::numeric_limits<I32>::max())) {
        return false;
    }
    ResourceGroup[resourceSort] = static_cast<I32>(next);
    return true;
}

bool Player::changeResource(I32 changeWoodAmount, I32 changeFoodAmount,
                            I32 changeStoneAmount, I32 changeGoldAmount)
{
    const array<I32, 4> delta = {changeWoodAmount, changeFoodAmount,
                                 changeStoneAmount, changeGoldAmount};

    // 要么全成功、要么全不动，避免"木够石不够"时只扣掉一半
    for (I32 i = 0; i < static_cast<I32>(ResourceGroup.size()); ++i) {
        const I64 next = static_cast<I64>(ResourceGroup[i]) + delta[i];
        if (next < 0 || next > static_cast<I64>(std::numeric_limits<I32>::max())) {
            return false;
        }
    }
    for (I32 i = 0; i < static_cast<I32>(ResourceGroup.size()); ++i) {
        ResourceGroup[i] = static_cast<I32>(static_cast<I64>(ResourceGroup[i]) + delta[i]);
    }
    return true;
}

/* ------------------------------------------------------------ 转化 */

bool Player::receiveArmy(SafePtr<Army> army)
{
    if (!army) {
        return false;
    }
    if (army->getBelong().second == teamID) {
        return false;   // 同阵营对象禁止转化
    }

    ReceivedObjectLockedDevelopment[army->getglobalNum()] =
        snapshotScience(army->getPlayerScience());
    army->setBelong(playerID, teamID);
    armies.push_back(army);
    return true;
}

bool Player::receiveFarmer(SafePtr<Farmer> farmer)
{
    if (!farmer) {
        return false;
    }
    if (farmer->getBelong().second == teamID) {
        return false;
    }

    ReceivedObjectLockedDevelopment[farmer->getglobalNum()] =
        snapshotScience(farmer->getPlayerScience());
    farmer->setBelong(playerID, teamID);
    farmers.push_back(farmer);
    return true;
}

bool Player::receiveBuilding(SafePtr<Building> building)
{
    if (!building) {
        return false;
    }
    if (building->getBelong().second == teamID) {
        return false;
    }

    ReceivedObjectLockedDevelopment[building->getglobalNum()] =
        snapshotScience(building->getPlayerScience());
    building->setBelong(playerID, teamID);
    buildings.push_back(building);
    return true;
}

bool Player::receiveBuildingResource(SafePtr<BuildingResource> buildingResource)
{
    if (!buildingResource) {
        return false;
    }
    if (buildingResource->getBelong().second == teamID) {
        return false;
    }

    ReceivedObjectLockedDevelopment[buildingResource->getglobalNum()] =
        snapshotScience(buildingResource->getPlayerScience());
    buildingResource->setBelong(playerID, teamID);
    buildingResources.push_back(buildingResource);
    return true;
}

/* ------------------------------------------------------------ 帧末收尾 */

void Player::tick()
{
    // 调用时机：一帧内所有对象行为（移动、攻击结算、死亡标记）都跑完之后。
    // 这里释放掉的死亡对象，本帧之后任何人都不能再持有裸指针。
    collectDead(armies, ReceivedObjectLockedDevelopment);
    collectDead(farmers, ReceivedObjectLockedDevelopment);
    collectDead(buildings, ReceivedObjectLockedDevelopment);
    collectDead(buildingResources, ReceivedObjectLockedDevelopment);

    // 投掷物没有血量，什么时候消失由内核（命中 / 超时）决定，
    // 这里只清理空槽位，不主动判定死亡。
    for (auto it = missiles.begin(); it != missiles.end();) {
        it = (*it) ? it + 1 : missiles.erase(it);
    }

    // 兜底：科技快照表里的每一项都必须对应一个还活着的对象。
    // 正常路径（removeXxx / 上面的 collectDead）已经同步删除，但 getXXX() 暴露的是
    // 可变引用，万一有调用方绕过接口直接从池子里 erase，map 里那份 SafePtr<Development>
    // 就会永久吊住一个科技表。这里每帧做一次对账，把孤儿项释放掉。
    if (!ReceivedObjectLockedDevelopment.empty()) {
        std::set<I32> alive;
        auto collect = [&alive](auto& pool) {
            for (auto& p : pool) {
                if (p) {
                    alive.insert(p->getglobalNum());
                }
            }
        };
        collect(armies);
        collect(farmers);
        collect(buildings);
        collect(buildingResources);
        collect(missiles);

        for (auto it = ReceivedObjectLockedDevelopment.begin();
             it != ReceivedObjectLockedDevelopment.end();) {
            if (alive.find(it->first) == alive.end()) {
                it = ReceivedObjectLockedDevelopment.erase(it);
            } else {
                ++it;
            }
        }
    }
}
