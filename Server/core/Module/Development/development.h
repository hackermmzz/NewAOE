#pragma once

#include "Server/core/type.h"
#include "Server/core/BaseObject/WeakPointer.hpp"

class Player;

class Development
{ 
public:
    // 构造函数：civilization 为文明类型（共 16 个，见 Civilization.h）
    Development(WeakPtr<Player> player, I32 civilization);
    ~Development() = default;


    // ---- 加成查询 ----
    // 返回某个 sort 下具体 type 的速度加成比例
    [[nodiscard]] Double get_rate_Move(I32 sort, I32 type) const;

    // 返回某个 sort 下具体 type 的血量加成比例
    [[nodiscard]] Double get_rate_Blood(I32 sort, I32 type) const;

    // 返回某个 sort 下具体 type 的额外血量加成
    [[nodiscard]] I32 get_addition_Blood(I32 sort, I32 type) const;

    // 返回某个 sort 下具体 type 的攻击加成比例（armyClass：骑兵/弓兵等）
    [[nodiscard]] Double get_rate_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;

    // 返回某个 sort 下具体 type 的攻击数值加成
    [[nodiscard]] I32 get_addition_Attack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;

    // 返回某个 sort 下具体 type 的攻击距离加成
    [[nodiscard]] I32 get_addition_DisAttack(I32 sort, I32 type, I32 armyClass, I32 attackType) const;

    // 返回某个 sort 下具体 type 的防御比例加成（attackType_got：受到攻击的类型）
    [[nodiscard]] Double get_rate_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const;

    // 返回某个 sort 下具体 type 的防御数值加成
    [[nodiscard]] I32 get_addition_Defence(I32 sort, I32 type, I32 armyClass, I32 attackType_got) const;

    // 资源可携带的额外数值加成
    [[nodiscard]] I32 get_addition_ResourceCarry(I32 resourceSort) const;

    // 资源采摘速度额外加成比例
    [[nodiscard]] Double get_rate_ResourceGather(I32 resourceSort) const;

    // 资源型建筑额外数量加成
    [[nodiscard]] I32 get_addition_BuildingResourceMaxCnt(I32 sort, I32 type) const;

    // ---- 时代 ----
    // 升级时代，自动对时代升级 +1
    void stageChange();

    // 是否满足时代升级的条件
    [[nodiscard]] bool stageCanChange() const;

    // ---- 建筑 ----
    // 是否允许建筑进行某个动作（参数为当前资源量）
    [[nodiscard]] bool buildingActionAble(I32 buildingType, I32 actNum,
                                          I32 wood, I32 food, I32 stone, I32 gold) const;

    // 建筑进行某个行动的耗时，返回帧数（非物理时间）
    [[nodiscard]] I32 buildingActionTimeCost(I32 buildingType, I32 actNum) const;

    // 判断建筑是否允许建造（需结合文明、阶段）
    [[nodiscard]] bool buildingCanBuild(I32 buildingType) const;

    // 建造某个建筑的耗时，返回帧数
    [[nodiscard]] I32 buildingBuildTimeCost(I32 buildingType) const;

    // 每次完整运行完一次内核后执行
    void tick();

private:
    WeakPtr<Player> player;   // 只能读 player 内元素，不得修改 player 的任何值
    I32 stage;                // 当前阶段（石器时代等）
    I32 civilization;         // 文明类型（共 16 个）

};