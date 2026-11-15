#pragma once

#include <array>

#include "Server/core/type.h"
#include "Server/core/BaseObject/WeakPointer.hpp"

class Player;
class Object;

// 玩家的科技/发展状态。私有成员只有 player / stage / civilization，
// 其中 player 仅用于只读查询，不得借它修改 Player 的任何状态。
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
    // 是否满足时代升级的条件
    [[nodiscard]] bool stageCanChange() const;

    // ---- 建筑 ----
    // 判断建筑是否允许建造（需结合文明、阶段）
    [[nodiscard]] bool buildingCanBuild(I32 buildingType) const;

    // 是否允许建筑进行某个动作
    [[nodiscard]] bool buildingActionAble(I32 buildingType, I32 actNum) const;

    // 建造某个建筑的耗时，返回帧数
    [[nodiscard]] I32 buildingBuildTimeCost(I32 buildingType) const;

    // 建造某个建筑需要的资源，顺序为木材、粮食、石头、黄金
    [[nodiscard]] std::array<I32,4> buildingResourceCost(I32 buildingType) const;

    // 建筑进行某个行动的耗时，返回帧数（非物理时间）
    [[nodiscard]] I32 buildingActionTimeCost(I32 buildingType, I32 actNum) const;

    // 建筑进行某个行动需要的资源，顺序为木材、粮食、石头、黄金
    [[nodiscard]] std::array<I32,4> buildingActionResourceCost(I32 buildingType, I32 actNum) const;

    // 内核通知：某个建筑动作已成功完成。时代升级类动作在这里推进 stage。
    void notifyBuildingActionFinish(WeakPtr<Object> object, I32 action);

    // 每次完整运行完一次内核后执行
    void tick();

private:
    WeakPtr<Player> player;   // 只能读 player 内元素，不得修改 player 的任何值
    I32 stage;                // 当前阶段（取值见 NEW_CIVILIZATION_STAGE）
    I32 civilization;         // 文明类型（共 16 个）
};
