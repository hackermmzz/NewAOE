#include "unit_attribute.h"
#include "Public/vec/cast.hpp"
#include <OpenXLSX.hpp>
#include <map>
#include <tuple>
#include <stdexcept>
#include <string>

// =========================================================
// 1. 全局变量的定义
// =========================================================
// --- 科技全局变量 ---
I32 TOOLWORKING_COST_FOOD = 0; // 工具制造术 食物成本
I32 TOOLWORKING_COST_WOOD = 0; // 工具制造术 木材成本
I32 TOOLWORKING_COST_GOLD = 0; // 工具制造术 黄金成本
I32 TOOLWORKING_COST_STONE = 0; // 工具制造术 石材成本
I32 TOOLWORKING_RESEARCH_TIME = 0; // 工具制造术 研发时间

I32 METALWORKING_COST_FOOD = 0; // 金属加工术 食物成本
I32 METALWORKING_COST_WOOD = 0; // 金属加工术 木材成本
I32 METALWORKING_COST_GOLD = 0; // 金属加工术 黄金成本
I32 METALWORKING_COST_STONE = 0; // 金属加工术 石材成本
I32 METALWORKING_RESEARCH_TIME = 0; // 金属加工术 研发时间

I32 METALURGY_COST_FOOD = 0; // 冶金术 食物成本
I32 METALURGY_COST_WOOD = 0; // 冶金术 木材成本
I32 METALURGY_COST_GOLD = 0; // 冶金术 黄金成本
I32 METALURGY_COST_STONE = 0; // 冶金术 石材成本
I32 METALURGY_RESEARCH_TIME = 0; // 冶金术 研发时间

I32 BRONZE_SHIELD_COST_FOOD = 0; // 青铜盾 食物成本
I32 BRONZE_SHIELD_COST_WOOD = 0; // 青铜盾 木材成本
I32 BRONZE_SHIELD_COST_GOLD = 0; // 青铜盾 黄金成本
I32 BRONZE_SHIELD_COST_STONE = 0; // 青铜盾 石材成本
I32 BRONZE_SHIELD_RESEARCH_TIME = 0; // 青铜盾 研发时间

I32 IRON_SHIELD_COST_FOOD = 0; // 铁盾 食物成本
I32 IRON_SHIELD_COST_WOOD = 0; // 铁盾 木材成本
I32 IRON_SHIELD_COST_GOLD = 0; // 铁盾 黄金成本
I32 IRON_SHIELD_COST_STONE = 0; // 铁盾 石材成本
I32 IRON_SHIELD_RESEARCH_TIME = 0; // 铁盾 研发时间

I32 TOWER_SHIELD_COST_FOOD = 0; // 塔盾 食物成本
I32 TOWER_SHIELD_COST_WOOD = 0; // 塔盾 木材成本
I32 TOWER_SHIELD_COST_GOLD = 0; // 塔盾 黄金成本
I32 TOWER_SHIELD_COST_STONE = 0; // 塔盾 石材成本
I32 TOWER_SHIELD_RESEARCH_TIME = 0; // 塔盾 研发时间

// --- 时代全局变量 ---
I32 TOOL_AGE_COST_FOOD = 0; // 石器时代 食物成本
I32 TOOL_AGE_COST_WOOD = 0; // 石器时代 木材成本
I32 TOOL_AGE_COST_GOLD = 0; // 石器时代 黄金成本
I32 TOOL_AGE_COST_STONE = 0; // 石器时代 石材成本
I32 TOOL_AGE_RESEARCH_TIME = 0; // 石器时代 研发时间

I32 BRONZE_AGE_COST_FOOD = 0; // 青铜时代 食物成本
I32 BRONZE_AGE_COST_WOOD = 0; // 青铜时代 木材成本
I32 BRONZE_AGE_COST_GOLD = 0; // 青铜时代 黄金成本
I32 BRONZE_AGE_COST_STONE = 0; // 青铜时代 石材成本
I32 BRONZE_AGE_RESEARCH_TIME = 0; // 青铜时代 研发时间

I32 IRON_AGE_COST_FOOD = 0; // 铁器时代 食物成本
I32 IRON_AGE_COST_WOOD = 0; // 铁器时代 木材成本
I32 IRON_AGE_COST_GOLD = 0; // 铁器时代 黄金成本
I32 IRON_AGE_COST_STONE = 0; // 铁器时代 石材成本
I32 IRON_AGE_RESEARCH_TIME = 0; // 铁器时代 研发时间

// --- 建筑全局变量 ---
I32 TOWN_CENTER_COST_FOOD = 0; // 城镇中心 食物花费
I32 TOWN_CENTER_COST_WOOD = 0; // 城镇中心 木材花费
I32 TOWN_CENTER_COST_GOLD = 0; // 城镇中心 黄金花费
I32 TOWN_CENTER_COST_STONE = 0; // 城镇中心 石材花费
I32 TOWN_CENTER_BUILD_TIME = 0; // 城镇中心 建造时间
I32 TOWN_CENTER_HP = 0; // 城镇中心 生命值

I32 HOUSE_COST_FOOD = 0; // 房屋 食物花费
I32 HOUSE_COST_WOOD = 0; // 房屋 木材花费
I32 HOUSE_COST_GOLD = 0; // 房屋 黄金花费
I32 HOUSE_COST_STONE = 0; // 房屋 石材花费
I32 HOUSE_BUILD_TIME = 0; // 房屋 建造时间
I32 HOUSE_HP = 0; // 房屋 生命值

I32 BARRACKS_COST_FOOD = 0; // 兵营 食物花费
I32 BARRACKS_COST_WOOD = 0; // 兵营 木材花费
I32 BARRACKS_COST_GOLD = 0; // 兵营 黄金花费
I32 BARRACKS_COST_STONE = 0; // 兵营 石材花费
I32 BARRACKS_BUILD_TIME = 0; // 兵营 建造时间
I32 BARRACKS_HP = 0; // 兵营 生命值

// --- 步兵全局变量 ---
I32 CLUBMAN_COST_FOOD = 0; // 石棍兵 食物成本
I32 CLUBMAN_COST_WOOD = 0; // 石棍兵 木材成本
I32 CLUBMAN_COST_GOLD = 0; // 石棍兵 黄金成本
I32 CLUBMAN_COST_STONE = 0; // 石棍兵 石材成本
I32 CLUBMAN_TRAIN_TIME = 0; // 石棍兵 生产时间
I32 CLUBMAN_HP = 0; // 石棍兵 生命值
Double CLUBMAN_SPEED = Double(0); // 石棍兵 移动速度
I32 CLUBMAN_LINE_OF_SIGHT = 0; // 石棍兵 视野
I32 CLUBMAN_HAND_ARMOR = 0; // 石棍兵 肉搏护甲
I32 CLUBMAN_PIERCE_ARMOR = 0; // 石棍兵 投射护甲
Double CLUBMAN_ATTACK_INTERVAL = Double(0); // 石棍兵 攻击间距
I32 CLUBMAN_ATTACK = 0; // 石棍兵 攻击力

I32 AXEMAN_COST_FOOD = 0; // 斧兵 食物成本
I32 AXEMAN_COST_WOOD = 0; // 斧兵 木材成本
I32 AXEMAN_COST_GOLD = 0; // 斧兵 黄金成本
I32 AXEMAN_COST_STONE = 0; // 斧兵 石材成本
I32 AXEMAN_TRAIN_TIME = 0; // 斧兵 生产时间
I32 AXEMAN_HP = 0; // 斧兵 生命值
Double AXEMAN_SPEED = Double(0); // 斧兵 移动速度
I32 AXEMAN_LINE_OF_SIGHT = 0; // 斧兵 视野
I32 AXEMAN_HAND_ARMOR = 0; // 斧兵 肉搏护甲
I32 AXEMAN_PIERCE_ARMOR = 0; // 斧兵 投射护甲
Double AXEMAN_ATTACK_INTERVAL = Double(0); // 斧兵 攻击间距
I32 AXEMAN_ATTACK = 0; // 斧兵 攻击力

I32 SLINGER_COST_FOOD = 0; // 投石兵 食物成本
I32 SLINGER_COST_WOOD = 0; // 投石兵 木材成本
I32 SLINGER_COST_GOLD = 0; // 投石兵 黄金成本
I32 SLINGER_COST_STONE = 0; // 投石兵 石材成本
I32 SLINGER_TRAIN_TIME = 0; // 投石兵 生产时间
I32 SLINGER_HP = 0; // 投石兵 生命值
Double SLINGER_SPEED = Double(0); // 投石兵 移动速度
I32 SLINGER_LINE_OF_SIGHT = 0; // 投石兵 视野
I32 SLINGER_HAND_ARMOR = 0; // 投石兵 肉搏护甲
I32 SLINGER_PIERCE_ARMOR = 0; // 投石兵 投射护甲
I32 SLINGER_RANGE = 0; // 投石兵 攻击距离
Double SLINGER_ATTACK_INTERVAL = Double(0); // 投石兵 攻击间距
I32 SLINGER_ATTACK = 0; // 投石兵 攻击力

I32 SHORT_SWORDSMAN_COST_FOOD = 0; // 短剑兵 食物成本
I32 SHORT_SWORDSMAN_COST_WOOD = 0; // 短剑兵 木材成本
I32 SHORT_SWORDSMAN_COST_GOLD = 0; // 短剑兵 黄金成本
I32 SHORT_SWORDSMAN_COST_STONE = 0; // 短剑兵 石材成本
I32 SHORT_SWORDSMAN_TRAIN_TIME = 0; // 短剑兵 生产时间
I32 SHORT_SWORDSMAN_HP = 0; // 短剑兵 生命值
Double SHORT_SWORDSMAN_SPEED = Double(0); // 短剑兵 移动速度
I32 SHORT_SWORDSMAN_LINE_OF_SIGHT = 0; // 短剑兵 视野
I32 SHORT_SWORDSMAN_HAND_ARMOR = 0; // 短剑兵 肉搏护甲
I32 SHORT_SWORDSMAN_PIERCE_ARMOR = 0; // 短剑兵 投射护甲
Double SHORT_SWORDSMAN_ATTACK_INTERVAL = Double(0); // 短剑兵 攻击间距
I32 SHORT_SWORDSMAN_ATTACK = 0; // 短剑兵 攻击力

I32 BROAD_SWORDSMAN_COST_FOOD = 0; // 阔剑兵 食物成本
I32 BROAD_SWORDSMAN_COST_WOOD = 0; // 阔剑兵 木材成本
I32 BROAD_SWORDSMAN_COST_GOLD = 0; // 阔剑兵 黄金成本
I32 BROAD_SWORDSMAN_COST_STONE = 0; // 阔剑兵 石材成本
I32 BROAD_SWORDSMAN_TRAIN_TIME = 0; // 阔剑兵 生产时间
I32 BROAD_SWORDSMAN_HP = 0; // 阔剑兵 生命值
Double BROAD_SWORDSMAN_SPEED = Double(0); // 阔剑兵 移动速度
I32 BROAD_SWORDSMAN_LINE_OF_SIGHT = 0; // 阔剑兵 视野
I32 BROAD_SWORDSMAN_HAND_ARMOR = 0; // 阔剑兵 肉搏护甲
I32 BROAD_SWORDSMAN_PIERCE_ARMOR = 0; // 阔剑兵 投射护甲
Double BROAD_SWORDSMAN_ATTACK_INTERVAL = Double(0); // 阔剑兵 攻击间距
I32 BROAD_SWORDSMAN_ATTACK = 0; // 阔剑兵 攻击力

I32 LONG_SWORDSMAN_COST_FOOD = 0; // 长剑兵 食物成本
I32 LONG_SWORDSMAN_COST_WOOD = 0; // 长剑兵 木材成本
I32 LONG_SWORDSMAN_COST_GOLD = 0; // 长剑兵 黄金成本
I32 LONG_SWORDSMAN_COST_STONE = 0; // 长剑兵 石材成本
I32 LONG_SWORDSMAN_TRAIN_TIME = 0; // 长剑兵 生产时间
I32 LONG_SWORDSMAN_HP = 0; // 长剑兵 生命值
Double LONG_SWORDSMAN_SPEED = Double(0); // 长剑兵 移动速度
I32 LONG_SWORDSMAN_LINE_OF_SIGHT = 0; // 长剑兵 视野
I32 LONG_SWORDSMAN_HAND_ARMOR = 0; // 长剑兵 肉搏护甲
I32 LONG_SWORDSMAN_PIERCE_ARMOR = 0; // 长剑兵 投射护甲
Double LONG_SWORDSMAN_ATTACK_INTERVAL = Double(0); // 长剑兵 攻击间距
I32 LONG_SWORDSMAN_ATTACK = 0; // 长剑兵 攻击力

I32 LEGION_COST_FOOD = 0; // 军团兵 食物成本
I32 LEGION_COST_WOOD = 0; // 军团兵 木材成本
I32 LEGION_COST_GOLD = 0; // 军团兵 黄金成本
I32 LEGION_COST_STONE = 0; // 军团兵 石材成本
I32 LEGION_TRAIN_TIME = 0; // 军团兵 生产时间
I32 LEGION_HP = 0; // 军团兵 生命值
Double LEGION_SPEED = Double(0); // 军团兵 移动速度
I32 LEGION_LINE_OF_SIGHT = 0; // 军团兵 视野
I32 LEGION_HAND_ARMOR = 0; // 军团兵 肉搏护甲
I32 LEGION_PIERCE_ARMOR = 0; // 军团兵 投射护甲
Double LEGION_ATTACK_INTERVAL = Double(0); // 军团兵 攻击间距
I32 LEGION_ATTACK = 0; // 军团兵 攻击力

I32 HOPLITE_COST_FOOD = 0; // 重装步兵 食物成本
I32 HOPLITE_COST_WOOD = 0; // 重装步兵 木材成本
I32 HOPLITE_COST_GOLD = 0; // 重装步兵 黄金成本
I32 HOPLITE_COST_STONE = 0; // 重装步兵 石材成本
I32 HOPLITE_TRAIN_TIME = 0; // 重装步兵 生产时间
I32 HOPLITE_HP = 0; // 重装步兵 生命值
Double HOPLITE_SPEED = Double(0); // 重装步兵 移动速度
I32 HOPLITE_LINE_OF_SIGHT = 0; // 重装步兵 视野
I32 HOPLITE_HAND_ARMOR = 0; // 重装步兵 肉搏护甲
I32 HOPLITE_PIERCE_ARMOR = 0; // 重装步兵 投射护甲
Double HOPLITE_ATTACK_INTERVAL = Double(0); // 重装步兵 攻击间距
I32 HOPLITE_ATTACK = 0; // 重装步兵 攻击力

I32 PHALANX_COST_FOOD = 0; // 方阵部队 食物成本
I32 PHALANX_COST_WOOD = 0; // 方阵部队 木材成本
I32 PHALANX_COST_GOLD = 0; // 方阵部队 黄金成本
I32 PHALANX_COST_STONE = 0; // 方阵部队 石材成本
I32 PHALANX_TRAIN_TIME = 0; // 方阵部队 生产时间
I32 PHALANX_HP = 0; // 方阵部队 生命值
Double PHALANX_SPEED = Double(0); // 方阵部队 移动速度
I32 PHALANX_LINE_OF_SIGHT = 0; // 方阵部队 视野
I32 PHALANX_HAND_ARMOR = 0; // 方阵部队 肉搏护甲
I32 PHALANX_PIERCE_ARMOR = 0; // 方阵部队 投射护甲
Double PHALANX_ATTACK_INTERVAL = Double(0); // 方阵部队 攻击间距
I32 PHALANX_ATTACK = 0; // 方阵部队 攻击力

I32 CENTURION_COST_FOOD = 0; // 百夫长 食物成本
I32 CENTURION_COST_WOOD = 0; // 百夫长 木材成本
I32 CENTURION_COST_GOLD = 0; // 百夫长 黄金成本
I32 CENTURION_COST_STONE = 0; // 百夫长 石材成本
I32 CENTURION_TRAIN_TIME = 0; // 百夫长 生产时间
I32 CENTURION_HP = 0; // 百夫长 生命值
Double CENTURION_SPEED = Double(0); // 百夫长 移动速度
I32 CENTURION_LINE_OF_SIGHT = 0; // 百夫长 视野
I32 CENTURION_HAND_ARMOR = 0; // 百夫长 肉搏护甲
I32 CENTURION_PIERCE_ARMOR = 0; // 百夫长 投射护甲
Double CENTURION_ATTACK_INTERVAL = Double(0); // 百夫长 攻击间距
I32 CENTURION_ATTACK = 0; // 百夫长 攻击力

// --- 弓兵全局变量 ---
I32 BOWMAN_COST_FOOD = 0; // 弓箭兵 食物成本
I32 BOWMAN_COST_WOOD = 0; // 弓箭兵 木材成本
I32 BOWMAN_COST_GOLD = 0; // 弓箭兵 黄金成本
I32 BOWMAN_COST_STONE = 0; // 弓箭兵 石材成本
I32 BOWMAN_TRAIN_TIME = 0; // 弓箭兵 生产时间
I32 BOWMAN_HP = 0; // 弓箭兵 生命值
Double BOWMAN_SPEED = Double(0); // 弓箭兵 移动速度
I32 BOWMAN_LINE_OF_SIGHT = 0; // 弓箭兵 视野
I32 BOWMAN_HAND_ARMOR = 0; // 弓箭兵 肉搏护甲
I32 BOWMAN_PIERCE_ARMOR = 0; // 弓箭兵 投射护甲
I32 BOWMAN_RANGE = 0; // 弓箭兵 攻击距离
Double BOWMAN_ATTACK_INTERVAL = Double(0); // 弓箭兵 攻击间距
I32 BOWMAN_ATTACK = 0; // 弓箭兵 攻击力

I32 IMPROVED_BOWMAN_COST_FOOD = 0; // 强弓兵 食物成本
I32 IMPROVED_BOWMAN_COST_WOOD = 0; // 强弓兵 木材成本
I32 IMPROVED_BOWMAN_COST_GOLD = 0; // 强弓兵 黄金成本
I32 IMPROVED_BOWMAN_COST_STONE = 0; // 强弓兵 石材成本
I32 IMPROVED_BOWMAN_TRAIN_TIME = 0; // 强弓兵 生产时间
I32 IMPROVED_BOWMAN_HP = 0; // 强弓兵 生命值
Double IMPROVED_BOWMAN_SPEED = Double(0); // 强弓兵 移动速度
I32 IMPROVED_BOWMAN_LINE_OF_SIGHT = 0; // 强弓兵 视野
I32 IMPROVED_BOWMAN_HAND_ARMOR = 0; // 强弓兵 肉搏护甲
I32 IMPROVED_BOWMAN_PIERCE_ARMOR = 0; // 强弓兵 投射护甲
I32 IMPROVED_BOWMAN_RANGE = 0; // 强弓兵 攻击距离
Double IMPROVED_BOWMAN_ATTACK_INTERVAL = Double(0); // 强弓兵 攻击间距
I32 IMPROVED_BOWMAN_ATTACK = 0; // 强弓兵 攻击力

I32 COMPOSITE_BOWMAN_COST_FOOD = 0; // 混合弓兵 食物成本
I32 COMPOSITE_BOWMAN_COST_WOOD = 0; // 混合弓兵 木材成本
I32 COMPOSITE_BOWMAN_COST_GOLD = 0; // 混合弓兵 黄金成本
I32 COMPOSITE_BOWMAN_COST_STONE = 0; // 混合弓兵 石材成本
I32 COMPOSITE_BOWMAN_TRAIN_TIME = 0; // 混合弓兵 生产时间
I32 COMPOSITE_BOWMAN_HP = 0; // 混合弓兵 生命值
Double COMPOSITE_BOWMAN_SPEED = Double(0); // 混合弓兵 移动速度
I32 COMPOSITE_BOWMAN_LINE_OF_SIGHT = 0; // 混合弓兵 视野
I32 COMPOSITE_BOWMAN_HAND_ARMOR = 0; // 混合弓兵 肉搏护甲
I32 COMPOSITE_BOWMAN_PIERCE_ARMOR = 0; // 混合弓兵 投射护甲
I32 COMPOSITE_BOWMAN_RANGE = 0; // 混合弓兵 攻击距离
Double COMPOSITE_BOWMAN_ATTACK_INTERVAL = Double(0); // 混合弓兵 攻击间距
I32 COMPOSITE_BOWMAN_ATTACK = 0; // 混合弓兵 攻击力

I32 CHARIOT_ARCHER_COST_FOOD = 0; // 战车弓兵 食物成本
I32 CHARIOT_ARCHER_COST_WOOD = 0; // 战车弓兵 木材成本
I32 CHARIOT_ARCHER_COST_GOLD = 0; // 战车弓兵 黄金成本
I32 CHARIOT_ARCHER_COST_STONE = 0; // 战车弓兵 石材成本
I32 CHARIOT_ARCHER_TRAIN_TIME = 0; // 战车弓兵 生产时间
I32 CHARIOT_ARCHER_HP = 0; // 战车弓兵 生命值
Double CHARIOT_ARCHER_SPEED = Double(0); // 战车弓兵 移动速度
I32 CHARIOT_ARCHER_LINE_OF_SIGHT = 0; // 战车弓兵 视野
I32 CHARIOT_ARCHER_HAND_ARMOR = 0; // 战车弓兵 肉搏护甲
I32 CHARIOT_ARCHER_PIERCE_ARMOR = 0; // 战车弓兵 投射护甲
I32 CHARIOT_ARCHER_RANGE = 0; // 战车弓兵 攻击距离
Double CHARIOT_ARCHER_ATTACK_INTERVAL = Double(0); // 战车弓兵 攻击间距
I32 CHARIOT_ARCHER_ATTACK = 0; // 战车弓兵 攻击力

I32 ELEPHANT_ARCHER_COST_FOOD = 0; // 战象弓兵 食物成本
I32 ELEPHANT_ARCHER_COST_WOOD = 0; // 战象弓兵 木材成本
I32 ELEPHANT_ARCHER_COST_GOLD = 0; // 战象弓兵 黄金成本
I32 ELEPHANT_ARCHER_COST_STONE = 0; // 战象弓兵 石材成本
I32 ELEPHANT_ARCHER_TRAIN_TIME = 0; // 战象弓兵 生产时间
I32 ELEPHANT_ARCHER_HP = 0; // 战象弓兵 生命值
Double ELEPHANT_ARCHER_SPEED = Double(0); // 战象弓兵 移动速度
I32 ELEPHANT_ARCHER_LINE_OF_SIGHT = 0; // 战象弓兵 视野
I32 ELEPHANT_ARCHER_HAND_ARMOR = 0; // 战象弓兵 肉搏护甲
I32 ELEPHANT_ARCHER_PIERCE_ARMOR = 0; // 战象弓兵 投射护甲
I32 ELEPHANT_ARCHER_RANGE = 0; // 战象弓兵 攻击距离
Double ELEPHANT_ARCHER_ATTACK_INTERVAL = Double(0); // 战象弓兵 攻击间距
I32 ELEPHANT_ARCHER_ATTACK = 0; // 战象弓兵 攻击力

I32 HORSE_ARCHER_COST_FOOD = 0; // 骑弓兵 食物成本
I32 HORSE_ARCHER_COST_WOOD = 0; // 骑弓兵 木材成本
I32 HORSE_ARCHER_COST_GOLD = 0; // 骑弓兵 黄金成本
I32 HORSE_ARCHER_COST_STONE = 0; // 骑弓兵 石材成本
I32 HORSE_ARCHER_TRAIN_TIME = 0; // 骑弓兵 生产时间
I32 HORSE_ARCHER_HP = 0; // 骑弓兵 生命值
Double HORSE_ARCHER_SPEED = Double(0); // 骑弓兵 移动速度
I32 HORSE_ARCHER_LINE_OF_SIGHT = 0; // 骑弓兵 视野
I32 HORSE_ARCHER_HAND_ARMOR = 0; // 骑弓兵 肉搏护甲
I32 HORSE_ARCHER_PIERCE_ARMOR = 0; // 骑弓兵 投射护甲
I32 HORSE_ARCHER_RANGE = 0; // 骑弓兵 攻击距离
Double HORSE_ARCHER_ATTACK_INTERVAL = Double(0); // 骑弓兵 攻击间距
I32 HORSE_ARCHER_ATTACK = 0; // 骑弓兵 攻击力

I32 HEAVY_HORSE_ARCHER_COST_FOOD = 0; // 重骑弓兵 食物成本
I32 HEAVY_HORSE_ARCHER_COST_WOOD = 0; // 重骑弓兵 木材成本
I32 HEAVY_HORSE_ARCHER_COST_GOLD = 0; // 重骑弓兵 黄金成本
I32 HEAVY_HORSE_ARCHER_COST_STONE = 0; // 重骑弓兵 石材成本
I32 HEAVY_HORSE_ARCHER_TRAIN_TIME = 0; // 重骑弓兵 生产时间
I32 HEAVY_HORSE_ARCHER_HP = 0; // 重骑弓兵 生命值
Double HEAVY_HORSE_ARCHER_SPEED = Double(0); // 重骑弓兵 移动速度
I32 HEAVY_HORSE_ARCHER_LINE_OF_SIGHT = 0; // 重骑弓兵 视野
I32 HEAVY_HORSE_ARCHER_HAND_ARMOR = 0; // 重骑弓兵 肉搏护甲
I32 HEAVY_HORSE_ARCHER_PIERCE_ARMOR = 0; // 重骑弓兵 投射护甲
I32 HEAVY_HORSE_ARCHER_RANGE = 0; // 重骑弓兵 攻击距离
Double HEAVY_HORSE_ARCHER_ATTACK_INTERVAL = Double(0); // 重骑弓兵 攻击间距
I32 HEAVY_HORSE_ARCHER_ATTACK = 0; // 重骑弓兵 攻击力

// --- 骑兵全局变量 ---
I32 SCOUT_COST_FOOD = 0; // 侦察骑兵 食物成本
I32 SCOUT_COST_WOOD = 0; // 侦察骑兵 木材成本
I32 SCOUT_COST_GOLD = 0; // 侦察骑兵 黄金成本
I32 SCOUT_COST_STONE = 0; // 侦察骑兵 石材成本
I32 SCOUT_TRAIN_TIME = 0; // 侦察骑兵 生产时间
I32 SCOUT_HP = 0; // 侦察骑兵 生命值
Double SCOUT_SPEED = Double(0); // 侦察骑兵 移动速度
I32 SCOUT_LINE_OF_SIGHT = 0; // 侦察骑兵 视野
I32 SCOUT_HAND_ARMOR = 0; // 侦察骑兵 肉搏护甲
I32 SCOUT_PIERCE_ARMOR = 0; // 侦察骑兵 投射护甲
Double SCOUT_ATTACK_INTERVAL = Double(0); // 侦察骑兵 攻击间距
I32 SCOUT_ATTACK = 0; // 侦察骑兵 攻击力

I32 CAMEL_RIDER_COST_FOOD = 0; // 骆驼兵 食物成本
I32 CAMEL_RIDER_COST_WOOD = 0; // 骆驼兵 木材成本
I32 CAMEL_RIDER_COST_GOLD = 0; // 骆驼兵 黄金成本
I32 CAMEL_RIDER_COST_STONE = 0; // 骆驼兵 石材成本
I32 CAMEL_RIDER_TRAIN_TIME = 0; // 骆驼兵 生产时间
I32 CAMEL_RIDER_HP = 0; // 骆驼兵 生命值
Double CAMEL_RIDER_SPEED = Double(0); // 骆驼兵 移动速度
I32 CAMEL_RIDER_LINE_OF_SIGHT = 0; // 骆驼兵 视野
I32 CAMEL_RIDER_HAND_ARMOR = 0; // 骆驼兵 肉搏护甲
I32 CAMEL_RIDER_PIERCE_ARMOR = 0; // 骆驼兵 投射护甲
Double CAMEL_RIDER_ATTACK_INTERVAL = Double(0); // 骆驼兵 攻击间距
I32 CAMEL_RIDER_ATTACK = 0; // 骆驼兵 攻击力

I32 CHARIOT_COST_FOOD = 0; // 两轮战车 食物成本
I32 CHARIOT_COST_WOOD = 0; // 两轮战车 木材成本
I32 CHARIOT_COST_GOLD = 0; // 两轮战车 黄金成本
I32 CHARIOT_COST_STONE = 0; // 两轮战车 石材成本
I32 CHARIOT_TRAIN_TIME = 0; // 两轮战车 生产时间
I32 CHARIOT_HP = 0; // 两轮战车 生命值
Double CHARIOT_SPEED = Double(0); // 两轮战车 移动速度
I32 CHARIOT_LINE_OF_SIGHT = 0; // 两轮战车 视野
I32 CHARIOT_HAND_ARMOR = 0; // 两轮战车 肉搏护甲
I32 CHARIOT_PIERCE_ARMOR = 0; // 两轮战车 投射护甲
Double CHARIOT_ATTACK_INTERVAL = Double(0); // 两轮战车 攻击间距
I32 CHARIOT_ATTACK = 0; // 两轮战车 攻击力

I32 SCYTHE_CHARIOT_COST_FOOD = 0; // 轮式镰刀战车 食物成本
I32 SCYTHE_CHARIOT_COST_WOOD = 0; // 轮式镰刀战车 木材成本
I32 SCYTHE_CHARIOT_COST_GOLD = 0; // 轮式镰刀战车 黄金成本
I32 SCYTHE_CHARIOT_COST_STONE = 0; // 轮式镰刀战车 石材成本
I32 SCYTHE_CHARIOT_TRAIN_TIME = 0; // 轮式镰刀战车 生产时间
I32 SCYTHE_CHARIOT_HP = 0; // 轮式镰刀战车 生命值
Double SCYTHE_CHARIOT_SPEED = Double(0); // 轮式镰刀战车 移动速度
I32 SCYTHE_CHARIOT_LINE_OF_SIGHT = 0; // 轮式镰刀战车 视野
I32 SCYTHE_CHARIOT_HAND_ARMOR = 0; // 轮式镰刀战车 肉搏护甲
I32 SCYTHE_CHARIOT_PIERCE_ARMOR = 0; // 轮式镰刀战车 投射护甲
Double SCYTHE_CHARIOT_ATTACK_INTERVAL = Double(0); // 轮式镰刀战车 攻击间距
I32 SCYTHE_CHARIOT_ATTACK = 0; // 轮式镰刀战车 攻击力

I32 CAVALRY_COST_FOOD = 0; // 骑兵 食物成本
I32 CAVALRY_COST_WOOD = 0; // 骑兵 木材成本
I32 CAVALRY_COST_GOLD = 0; // 骑兵 黄金成本
I32 CAVALRY_COST_STONE = 0; // 骑兵 石材成本
I32 CAVALRY_TRAIN_TIME = 0; // 骑兵 生产时间
I32 CAVALRY_HP = 0; // 骑兵 生命值
Double CAVALRY_SPEED = Double(0); // 骑兵 移动速度
I32 CAVALRY_LINE_OF_SIGHT = 0; // 骑兵 视野
I32 CAVALRY_HAND_ARMOR = 0; // 骑兵 肉搏护甲
I32 CAVALRY_PIERCE_ARMOR = 0; // 骑兵 投射护甲
Double CAVALRY_ATTACK_INTERVAL = Double(0); // 骑兵 攻击间距
I32 CAVALRY_ATTACK = 0; // 骑兵 攻击力

I32 HEAVY_CAVALRY_COST_FOOD = 0; // 重骑兵 食物成本
I32 HEAVY_CAVALRY_COST_WOOD = 0; // 重骑兵 木材成本
I32 HEAVY_CAVALRY_COST_GOLD = 0; // 重骑兵 黄金成本
I32 HEAVY_CAVALRY_COST_STONE = 0; // 重骑兵 石材成本
I32 HEAVY_CAVALRY_TRAIN_TIME = 0; // 重骑兵 生产时间
I32 HEAVY_CAVALRY_HP = 0; // 重骑兵 生命值
Double HEAVY_CAVALRY_SPEED = Double(0); // 重骑兵 移动速度
I32 HEAVY_CAVALRY_LINE_OF_SIGHT = 0; // 重骑兵 视野
I32 HEAVY_CAVALRY_HAND_ARMOR = 0; // 重骑兵 肉搏护甲
I32 HEAVY_CAVALRY_PIERCE_ARMOR = 0; // 重骑兵 投射护甲
Double HEAVY_CAVALRY_ATTACK_INTERVAL = Double(0); // 重骑兵 攻击间距
I32 HEAVY_CAVALRY_ATTACK = 0; // 重骑兵 攻击力

I32 CATAPHRACT_COST_FOOD = 0; // 圣骑兵 食物成本
I32 CATAPHRACT_COST_WOOD = 0; // 圣骑兵 木材成本
I32 CATAPHRACT_COST_GOLD = 0; // 圣骑兵 黄金成本
I32 CATAPHRACT_COST_STONE = 0; // 圣骑兵 石材成本
I32 CATAPHRACT_TRAIN_TIME = 0; // 圣骑兵 生产时间
I32 CATAPHRACT_HP = 0; // 圣骑兵 生命值
Double CATAPHRACT_SPEED = Double(0); // 圣骑兵 移动速度
I32 CATAPHRACT_LINE_OF_SIGHT = 0; // 圣骑兵 视野
I32 CATAPHRACT_HAND_ARMOR = 0; // 圣骑兵 肉搏护甲
I32 CATAPHRACT_PIERCE_ARMOR = 0; // 圣骑兵 投射护甲
Double CATAPHRACT_ATTACK_INTERVAL = Double(0); // 圣骑兵 攻击间距
I32 CATAPHRACT_ATTACK = 0; // 圣骑兵 攻击力

// --- 战象全局变量 ---
I32 WAR_ELEPHANT_COST_FOOD = 0; // 战象 食物成本
I32 WAR_ELEPHANT_COST_WOOD = 0; // 战象 木材成本
I32 WAR_ELEPHANT_COST_GOLD = 0; // 战象 黄金成本
I32 WAR_ELEPHANT_COST_STONE = 0; // 战象 石材成本
I32 WAR_ELEPHANT_TRAIN_TIME = 0; // 战象 生产时间
I32 WAR_ELEPHANT_HP = 0; // 战象 生命值
Double WAR_ELEPHANT_SPEED = Double(0); // 战象 移动速度
I32 WAR_ELEPHANT_LINE_OF_SIGHT = 0; // 战象 视野
I32 WAR_ELEPHANT_HAND_ARMOR = 0; // 战象 肉搏护甲
I32 WAR_ELEPHANT_PIERCE_ARMOR = 0; // 战象 投射护甲
Double WAR_ELEPHANT_ATTACK_INTERVAL = Double(0); // 战象 攻击间距
I32 WAR_ELEPHANT_ATTACK = 0; // 战象 攻击力

I32 ARMORED_ELEPHANT_COST_FOOD = 0; // 披甲战象 食物成本
I32 ARMORED_ELEPHANT_COST_WOOD = 0; // 披甲战象 木材成本
I32 ARMORED_ELEPHANT_COST_GOLD = 0; // 披甲战象 黄金成本
I32 ARMORED_ELEPHANT_COST_STONE = 0; // 披甲战象 石材成本
I32 ARMORED_ELEPHANT_TRAIN_TIME = 0; // 披甲战象 生产时间
I32 ARMORED_ELEPHANT_HP = 0; // 披甲战象 生命值
Double ARMORED_ELEPHANT_SPEED = Double(0); // 披甲战象 移动速度
I32 ARMORED_ELEPHANT_LINE_OF_SIGHT = 0; // 披甲战象 视野
I32 ARMORED_ELEPHANT_HAND_ARMOR = 0; // 披甲战象 肉搏护甲
I32 ARMORED_ELEPHANT_PIERCE_ARMOR = 0; // 披甲战象 投射护甲
Double ARMORED_ELEPHANT_ATTACK_INTERVAL = Double(0); // 披甲战象 攻击间距
I32 ARMORED_ELEPHANT_ATTACK = 0; // 披甲战象 攻击力

// --- 攻城武器全局变量 ---
I32 STONE_THROWER_COST_FOOD = 0; // 投石器 食物成本
I32 STONE_THROWER_COST_WOOD = 0; // 投石器 木材成本
I32 STONE_THROWER_COST_GOLD = 0; // 投石器 黄金成本
I32 STONE_THROWER_COST_STONE = 0; // 投石器 石材成本
I32 STONE_THROWER_TRAIN_TIME = 0; // 投石器 生产时间
I32 STONE_THROWER_HP = 0; // 投石器 生命值
Double STONE_THROWER_SPEED = Double(0); // 投石器 移动速度
I32 STONE_THROWER_LINE_OF_SIGHT = 0; // 投石器 视野
I32 STONE_THROWER_HAND_ARMOR = 0; // 投石器 肉搏护甲
I32 STONE_THROWER_PIERCE_ARMOR = 0; // 投石器 投射护甲
I32 STONE_THROWER_RANGE = 0; // 投石器 攻击距离
Double STONE_THROWER_ATTACK_INTERVAL = Double(0); // 投石器 攻击间距
I32 STONE_THROWER_ATTACK = 0; // 投石器 攻击力

I32 CATAPULT_COST_FOOD = 0; // 投石车 食物成本
I32 CATAPULT_COST_WOOD = 0; // 投石车 木材成本
I32 CATAPULT_COST_GOLD = 0; // 投石车 黄金成本
I32 CATAPULT_COST_STONE = 0; // 投石车 石材成本
I32 CATAPULT_TRAIN_TIME = 0; // 投石车 生产时间
I32 CATAPULT_HP = 0; // 投石车 生命值
Double CATAPULT_SPEED = Double(0); // 投石车 移动速度
I32 CATAPULT_LINE_OF_SIGHT = 0; // 投石车 视野
I32 CATAPULT_HAND_ARMOR = 0; // 投石车 肉搏护甲
I32 CATAPULT_PIERCE_ARMOR = 0; // 投石车 投射护甲
I32 CATAPULT_RANGE = 0; // 投石车 攻击距离
Double CATAPULT_ATTACK_INTERVAL = Double(0); // 投石车 攻击间距
I32 CATAPULT_ATTACK = 0; // 投石车 攻击力

I32 HEAVY_CATAPULT_COST_FOOD = 0; // 重型投石车 食物成本
I32 HEAVY_CATAPULT_COST_WOOD = 0; // 重型投石车 木材成本
I32 HEAVY_CATAPULT_COST_GOLD = 0; // 重型投石车 黄金成本
I32 HEAVY_CATAPULT_COST_STONE = 0; // 重型投石车 石材成本
I32 HEAVY_CATAPULT_TRAIN_TIME = 0; // 重型投石车 生产时间
I32 HEAVY_CATAPULT_HP = 0; // 重型投石车 生命值
Double HEAVY_CATAPULT_SPEED = Double(0); // 重型投石车 移动速度
I32 HEAVY_CATAPULT_LINE_OF_SIGHT = 0; // 重型投石车 视野
I32 HEAVY_CATAPULT_HAND_ARMOR = 0; // 重型投石车 肉搏护甲
I32 HEAVY_CATAPULT_PIERCE_ARMOR = 0; // 重型投石车 投射护甲
I32 HEAVY_CATAPULT_RANGE = 0; // 重型投石车 攻击距离
Double HEAVY_CATAPULT_ATTACK_INTERVAL = Double(0); // 重型投石车 攻击间距
I32 HEAVY_CATAPULT_ATTACK = 0; // 重型投石车 攻击力

I32 BALLISTA_COST_FOOD = 0; // 弩车 食物成本
I32 BALLISTA_COST_WOOD = 0; // 弩车 木材成本
I32 BALLISTA_COST_GOLD = 0; // 弩车 黄金成本
I32 BALLISTA_COST_STONE = 0; // 弩车 石材成本
I32 BALLISTA_TRAIN_TIME = 0; // 弩车 生产时间
I32 BALLISTA_HP = 0; // 弩车 生命值
Double BALLISTA_SPEED = Double(0); // 弩车 移动速度
I32 BALLISTA_LINE_OF_SIGHT = 0; // 弩车 视野
I32 BALLISTA_HAND_ARMOR = 0; // 弩车 肉搏护甲
I32 BALLISTA_PIERCE_ARMOR = 0; // 弩车 投射护甲
I32 BALLISTA_RANGE = 0; // 弩车 攻击距离
Double BALLISTA_ATTACK_INTERVAL = Double(0); // 弩车 攻击间距
I32 BALLISTA_ATTACK = 0; // 弩车 攻击力

I32 HELEPOLIS_COST_FOOD = 0; // 巨型弩车 食物成本
I32 HELEPOLIS_COST_WOOD = 0; // 巨型弩车 木材成本
I32 HELEPOLIS_COST_GOLD = 0; // 巨型弩车 黄金成本
I32 HELEPOLIS_COST_STONE = 0; // 巨型弩车 石材成本
I32 HELEPOLIS_TRAIN_TIME = 0; // 巨型弩车 生产时间
I32 HELEPOLIS_HP = 0; // 巨型弩车 生命值
Double HELEPOLIS_SPEED = Double(0); // 巨型弩车 移动速度
I32 HELEPOLIS_LINE_OF_SIGHT = 0; // 巨型弩车 视野
I32 HELEPOLIS_HAND_ARMOR = 0; // 巨型弩车 肉搏护甲
I32 HELEPOLIS_PIERCE_ARMOR = 0; // 巨型弩车 投射护甲
I32 HELEPOLIS_RANGE = 0; // 巨型弩车 攻击距离
Double HELEPOLIS_ATTACK_INTERVAL = Double(0); // 巨型弩车 攻击间距
I32 HELEPOLIS_ATTACK = 0; // 巨型弩车 攻击力

// --- 船只全局变量 ---
I32 FISHING_BOAT_COST_FOOD = 0; // 小渔船 食物成本
I32 FISHING_BOAT_COST_WOOD = 0; // 小渔船 木材成本
I32 FISHING_BOAT_COST_GOLD = 0; // 小渔船 黄金成本
I32 FISHING_BOAT_COST_STONE = 0; // 小渔船 石材成本
I32 FISHING_BOAT_TRAIN_TIME = 0; // 小渔船 生产时间
I32 FISHING_BOAT_HP = 0; // 小渔船 生命值
Double FISHING_BOAT_SPEED = Double(0); // 小渔船 移动速度
I32 FISHING_BOAT_LINE_OF_SIGHT = 0; // 小渔船 视野
I32 FISHING_BOAT_HAND_ARMOR = 0; // 小渔船 肉搏护甲
I32 FISHING_BOAT_PIERCE_ARMOR = 0; // 小渔船 投射护甲

I32 FISHING_SHIP_COST_FOOD = 0; // 渔船 食物成本
I32 FISHING_SHIP_COST_WOOD = 0; // 渔船 木材成本
I32 FISHING_SHIP_COST_GOLD = 0; // 渔船 黄金成本
I32 FISHING_SHIP_COST_STONE = 0; // 渔船 石材成本
I32 FISHING_SHIP_TRAIN_TIME = 0; // 渔船 生产时间
I32 FISHING_SHIP_HP = 0; // 渔船 生命值
Double FISHING_SHIP_SPEED = Double(0); // 渔船 移动速度
I32 FISHING_SHIP_LINE_OF_SIGHT = 0; // 渔船 视野
I32 FISHING_SHIP_HAND_ARMOR = 0; // 渔船 肉搏护甲
I32 FISHING_SHIP_PIERCE_ARMOR = 0; // 渔船 投射护甲

I32 TRADE_BOAT_COST_FOOD = 0; // 小商船 食物成本
I32 TRADE_BOAT_COST_WOOD = 0; // 小商船 木材成本
I32 TRADE_BOAT_COST_GOLD = 0; // 小商船 黄金成本
I32 TRADE_BOAT_COST_STONE = 0; // 小商船 石材成本
I32 TRADE_BOAT_TRAIN_TIME = 0; // 小商船 生产时间
I32 TRADE_BOAT_HP = 0; // 小商船 生命值
Double TRADE_BOAT_SPEED = Double(0); // 小商船 移动速度
I32 TRADE_BOAT_LINE_OF_SIGHT = 0; // 小商船 视野
I32 TRADE_BOAT_HAND_ARMOR = 0; // 小商船 肉搏护甲
I32 TRADE_BOAT_PIERCE_ARMOR = 0; // 小商船 投射护甲

I32 MERCHANT_SHIP_COST_FOOD = 0; // 商船 食物成本
I32 MERCHANT_SHIP_COST_WOOD = 0; // 商船 木材成本
I32 MERCHANT_SHIP_COST_GOLD = 0; // 商船 黄金成本
I32 MERCHANT_SHIP_COST_STONE = 0; // 商船 石材成本
I32 MERCHANT_SHIP_TRAIN_TIME = 0; // 商船 生产时间
I32 MERCHANT_SHIP_HP = 0; // 商船 生命值
Double MERCHANT_SHIP_SPEED = Double(0); // 商船 移动速度
I32 MERCHANT_SHIP_LINE_OF_SIGHT = 0; // 商船 视野
I32 MERCHANT_SHIP_HAND_ARMOR = 0; // 商船 肉搏护甲
I32 MERCHANT_SHIP_PIERCE_ARMOR = 0; // 商船 投射护甲

I32 LIGHT_TRANSPORT_COST_FOOD = 0; // 小型运输船 食物成本
I32 LIGHT_TRANSPORT_COST_WOOD = 0; // 小型运输船 木材成本
I32 LIGHT_TRANSPORT_COST_GOLD = 0; // 小型运输船 黄金成本
I32 LIGHT_TRANSPORT_COST_STONE = 0; // 小型运输船 石材成本
I32 LIGHT_TRANSPORT_TRAIN_TIME = 0; // 小型运输船 生产时间
I32 LIGHT_TRANSPORT_HP = 0; // 小型运输船 生命值
Double LIGHT_TRANSPORT_SPEED = Double(0); // 小型运输船 移动速度
I32 LIGHT_TRANSPORT_LINE_OF_SIGHT = 0; // 小型运输船 视野
I32 LIGHT_TRANSPORT_HAND_ARMOR = 0; // 小型运输船 肉搏护甲
I32 LIGHT_TRANSPORT_PIERCE_ARMOR = 0; // 小型运输船 投射护甲

I32 HEAVY_TRANSPORT_COST_FOOD = 0; // 大型运输船 食物成本
I32 HEAVY_TRANSPORT_COST_WOOD = 0; // 大型运输船 木材成本
I32 HEAVY_TRANSPORT_COST_GOLD = 0; // 大型运输船 黄金成本
I32 HEAVY_TRANSPORT_COST_STONE = 0; // 大型运输船 石材成本
I32 HEAVY_TRANSPORT_TRAIN_TIME = 0; // 大型运输船 生产时间
I32 HEAVY_TRANSPORT_HP = 0; // 大型运输船 生命值
Double HEAVY_TRANSPORT_SPEED = Double(0); // 大型运输船 移动速度
I32 HEAVY_TRANSPORT_LINE_OF_SIGHT = 0; // 大型运输船 视野
I32 HEAVY_TRANSPORT_HAND_ARMOR = 0; // 大型运输船 肉搏护甲
I32 HEAVY_TRANSPORT_PIERCE_ARMOR = 0; // 大型运输船 投射护甲

I32 SCOUT_SHIP_COST_FOOD = 0; // 侦察船 食物成本
I32 SCOUT_SHIP_COST_WOOD = 0; // 侦察船 木材成本
I32 SCOUT_SHIP_COST_GOLD = 0; // 侦察船 黄金成本
I32 SCOUT_SHIP_COST_STONE = 0; // 侦察船 石材成本
I32 SCOUT_SHIP_TRAIN_TIME = 0; // 侦察船 生产时间
I32 SCOUT_SHIP_HP = 0; // 侦察船 生命值
Double SCOUT_SHIP_SPEED = Double(0); // 侦察船 移动速度
I32 SCOUT_SHIP_LINE_OF_SIGHT = 0; // 侦察船 视野
I32 SCOUT_SHIP_HAND_ARMOR = 0; // 侦察船 肉搏护甲
I32 SCOUT_SHIP_PIERCE_ARMOR = 0; // 侦察船 投射护甲
I32 SCOUT_SHIP_RANGE = 0; // 侦察船 攻击距离
Double SCOUT_SHIP_ATTACK_INTERVAL = Double(0); // 侦察船 攻击间距
I32 SCOUT_SHIP_ATTACK = 0; // 侦察船 攻击力

I32 WAR_GALLEY_COST_FOOD = 0; // 战船 食物成本
I32 WAR_GALLEY_COST_WOOD = 0; // 战船 木材成本
I32 WAR_GALLEY_COST_GOLD = 0; // 战船 黄金成本
I32 WAR_GALLEY_COST_STONE = 0; // 战船 石材成本
I32 WAR_GALLEY_TRAIN_TIME = 0; // 战船 生产时间
I32 WAR_GALLEY_HP = 0; // 战船 生命值
Double WAR_GALLEY_SPEED = Double(0); // 战船 移动速度
I32 WAR_GALLEY_LINE_OF_SIGHT = 0; // 战船 视野
I32 WAR_GALLEY_HAND_ARMOR = 0; // 战船 肉搏护甲
I32 WAR_GALLEY_PIERCE_ARMOR = 0; // 战船 投射护甲
I32 WAR_GALLEY_RANGE = 0; // 战船 攻击距离
Double WAR_GALLEY_ATTACK_INTERVAL = Double(0); // 战船 攻击间距
I32 WAR_GALLEY_ATTACK = 0; // 战船 攻击力

I32 FIRE_GALLEY_COST_FOOD = 0; // 喷火船 食物成本
I32 FIRE_GALLEY_COST_WOOD = 0; // 喷火船 木材成本
I32 FIRE_GALLEY_COST_GOLD = 0; // 喷火船 黄金成本
I32 FIRE_GALLEY_COST_STONE = 0; // 喷火船 石材成本
I32 FIRE_GALLEY_TRAIN_TIME = 0; // 喷火船 生产时间
I32 FIRE_GALLEY_HP = 0; // 喷火船 生命值
Double FIRE_GALLEY_SPEED = Double(0); // 喷火船 移动速度
I32 FIRE_GALLEY_LINE_OF_SIGHT = 0; // 喷火船 视野
I32 FIRE_GALLEY_HAND_ARMOR = 0; // 喷火船 肉搏护甲
I32 FIRE_GALLEY_PIERCE_ARMOR = 0; // 喷火船 投射护甲
I32 FIRE_GALLEY_RANGE = 0; // 喷火船 攻击距离
Double FIRE_GALLEY_ATTACK_INTERVAL = Double(0); // 喷火船 攻击间距
I32 FIRE_GALLEY_ATTACK = 0; // 喷火船 攻击力

I32 TRIREME_COST_FOOD = 0; // 三层桨座战船 食物成本
I32 TRIREME_COST_WOOD = 0; // 三层桨座战船 木材成本
I32 TRIREME_COST_GOLD = 0; // 三层桨座战船 黄金成本
I32 TRIREME_COST_STONE = 0; // 三层桨座战船 石材成本
I32 TRIREME_TRAIN_TIME = 0; // 三层桨座战船 生产时间
I32 TRIREME_HP = 0; // 三层桨座战船 生命值
Double TRIREME_SPEED = Double(0); // 三层桨座战船 移动速度
I32 TRIREME_LINE_OF_SIGHT = 0; // 三层桨座战船 视野
I32 TRIREME_HAND_ARMOR = 0; // 三层桨座战船 肉搏护甲
I32 TRIREME_PIERCE_ARMOR = 0; // 三层桨座战船 投射护甲
I32 TRIREME_RANGE = 0; // 三层桨座战船 攻击距离
Double TRIREME_ATTACK_INTERVAL = Double(0); // 三层桨座战船 攻击间距
I32 TRIREME_ATTACK = 0; // 三层桨座战船 攻击力

I32 CATAPULT_TRIREME_COST_FOOD = 0; // 投石战船 食物成本
I32 CATAPULT_TRIREME_COST_WOOD = 0; // 投石战船 木材成本
I32 CATAPULT_TRIREME_COST_GOLD = 0; // 投石战船 黄金成本
I32 CATAPULT_TRIREME_COST_STONE = 0; // 投石战船 石材成本
I32 CATAPULT_TRIREME_TRAIN_TIME = 0; // 投石战船 生产时间
I32 CATAPULT_TRIREME_HP = 0; // 投石战船 生命值
Double CATAPULT_TRIREME_SPEED = Double(0); // 投石战船 移动速度
I32 CATAPULT_TRIREME_LINE_OF_SIGHT = 0; // 投石战船 视野
I32 CATAPULT_TRIREME_HAND_ARMOR = 0; // 投石战船 肉搏护甲
I32 CATAPULT_TRIREME_PIERCE_ARMOR = 0; // 投石战船 投射护甲
I32 CATAPULT_TRIREME_RANGE = 0; // 投石战船 攻击距离
Double CATAPULT_TRIREME_ATTACK_INTERVAL = Double(0); // 投石战船 攻击间距
I32 CATAPULT_TRIREME_ATTACK = 0; // 投石战船 攻击力

I32 JUGGERNAUT_COST_FOOD = 0; // 巨型战舰 食物成本
I32 JUGGERNAUT_COST_WOOD = 0; // 巨型战舰 木材成本
I32 JUGGERNAUT_COST_GOLD = 0; // 巨型战舰 黄金成本
I32 JUGGERNAUT_COST_STONE = 0; // 巨型战舰 石材成本
I32 JUGGERNAUT_TRAIN_TIME = 0; // 巨型战舰 生产时间
I32 JUGGERNAUT_HP = 0; // 巨型战舰 生命值
Double JUGGERNAUT_SPEED = Double(0); // 巨型战舰 移动速度
I32 JUGGERNAUT_LINE_OF_SIGHT = 0; // 巨型战舰 视野
I32 JUGGERNAUT_HAND_ARMOR = 0; // 巨型战舰 肉搏护甲
I32 JUGGERNAUT_PIERCE_ARMOR = 0; // 巨型战舰 投射护甲
I32 JUGGERNAUT_RANGE = 0; // 巨型战舰 攻击距离
Double JUGGERNAUT_ATTACK_INTERVAL = Double(0); // 巨型战舰 攻击间距
I32 JUGGERNAUT_ATTACK = 0; // 巨型战舰 攻击力

// --- 防御塔全局变量 ---
I32 WATCH_TOWER_COST_FOOD = 0; // 瞭望塔 食物成本
I32 WATCH_TOWER_COST_WOOD = 0; // 瞭望塔 木材成本
I32 WATCH_TOWER_COST_GOLD = 0; // 瞭望塔 黄金成本
I32 WATCH_TOWER_COST_STONE = 0; // 瞭望塔 石材成本
I32 WATCH_TOWER_TRAIN_TIME = 0; // 瞭望塔 生产时间
I32 WATCH_TOWER_HP = 0; // 瞭望塔 生命值
I32 WATCH_TOWER_LINE_OF_SIGHT = 0; // 瞭望塔 视野
I32 WATCH_TOWER_PIERCE_ARMOR = 0; // 瞭望塔 投射护甲
I32 WATCH_TOWER_RANGE = 0; // 瞭望塔 攻击距离
Double WATCH_TOWER_ATTACK_INTERVAL = Double(0); // 瞭望塔 攻击间距
I32 WATCH_TOWER_ATTACK = 0; // 瞭望塔 攻击力

I32 SENTRY_TOWER_COST_FOOD = 0; // 哨塔 食物成本
I32 SENTRY_TOWER_COST_WOOD = 0; // 哨塔 木材成本
I32 SENTRY_TOWER_COST_GOLD = 0; // 哨塔 黄金成本
I32 SENTRY_TOWER_COST_STONE = 0; // 哨塔 石材成本
I32 SENTRY_TOWER_TRAIN_TIME = 0; // 哨塔 生产时间
I32 SENTRY_TOWER_HP = 0; // 哨塔 生命值
I32 SENTRY_TOWER_LINE_OF_SIGHT = 0; // 哨塔 视野
I32 SENTRY_TOWER_PIERCE_ARMOR = 0; // 哨塔 投射护甲
I32 SENTRY_TOWER_RANGE = 0; // 哨塔 攻击距离
Double SENTRY_TOWER_ATTACK_INTERVAL = Double(0); // 哨塔 攻击间距
I32 SENTRY_TOWER_ATTACK = 0; // 哨塔 攻击力

I32 GUARD_TOWER_COST_FOOD = 0; // 守卫塔 食物成本
I32 GUARD_TOWER_COST_WOOD = 0; // 守卫塔 木材成本
I32 GUARD_TOWER_COST_GOLD = 0; // 守卫塔 黄金成本
I32 GUARD_TOWER_COST_STONE = 0; // 守卫塔 石材成本
I32 GUARD_TOWER_TRAIN_TIME = 0; // 守卫塔 生产时间
I32 GUARD_TOWER_HP = 0; // 守卫塔 生命值
I32 GUARD_TOWER_LINE_OF_SIGHT = 0; // 守卫塔 视野
I32 GUARD_TOWER_PIERCE_ARMOR = 0; // 守卫塔 投射护甲
I32 GUARD_TOWER_RANGE = 0; // 守卫塔 攻击距离
Double GUARD_TOWER_ATTACK_INTERVAL = Double(0); // 守卫塔 攻击间距
I32 GUARD_TOWER_ATTACK = 0; // 守卫塔 攻击力

I32 BALLISTA_TOWER_COST_FOOD = 0; // 弩炮塔 食物成本
I32 BALLISTA_TOWER_COST_WOOD = 0; // 弩炮塔 木材成本
I32 BALLISTA_TOWER_COST_GOLD = 0; // 弩炮塔 黄金成本
I32 BALLISTA_TOWER_COST_STONE = 0; // 弩炮塔 石材成本
I32 BALLISTA_TOWER_TRAIN_TIME = 0; // 弩炮塔 生产时间
I32 BALLISTA_TOWER_HP = 0; // 弩炮塔 生命值
I32 BALLISTA_TOWER_LINE_OF_SIGHT = 0; // 弩炮塔 视野
I32 BALLISTA_TOWER_PIERCE_ARMOR = 0; // 弩炮塔 投射护甲
I32 BALLISTA_TOWER_RANGE = 0; // 弩炮塔 攻击距离
Double BALLISTA_TOWER_ATTACK_INTERVAL = Double(0); // 弩炮塔 攻击间距
I32 BALLISTA_TOWER_ATTACK = 0; // 弩炮塔 攻击力


// =========================================================
// 2. 读取解析函数实现
// =========================================================
void ParseUnitAttribute(const char* filepath) {
    if (filepath == nullptr || *filepath == '\0') {
        throw std::invalid_argument("Excel 文件路径为空");
    }

    OpenXLSX::XLDocument document;
    document.open(filepath);
    auto sheet = document.workbook().worksheet(1);

    // 读取单元格的 Lambda
    auto readString = [&](uint32_t row, uint16_t column) -> std::string {
        const OpenXLSX::XLCellValue cellValue = sheet.cell(row, column).value();
        if (cellValue.type() == OpenXLSX::XLValueType::Empty) return {};
        if (cellValue.type() != OpenXLSX::XLValueType::String) {
            throw std::runtime_error("Excel 第 " + std::to_string(row) + " 行第 " + std::to_string(column) + " 列不是字符串类型");
        }
        return cellValue.get<std::string>();
    };

    // 校验表头
    if (readString(1, 1) != "Key" || readString(1, 2) != "Value" || readString(1, 3) != "Description") {
        throw std::runtime_error("Excel 表头必须是 Key、Value、Description");
    }

    // 将 Excel 数据读入 Map
    std::map<std::string, std::tuple<std::string, std::string>> result;
    for (uint32_t row = 2; row <= sheet.rowCount(); ++row) {
        std::string key = readString(row, 1);
        std::string value = readString(row, 2);
        std::string description = readString(row, 3);

        if (key.empty() && value.empty() && description.empty()) continue;
        if (key.empty()) throw std::runtime_error("Excel 第 " + std::to_string(row) + " 行的 Key 为空");
        result[key] = std::make_tuple(std::move(value), std::move(description));
    }
    document.close();

    // =========================================================
    // 3. 数据赋值逻辑
    // =========================================================
    // 科技
    if (result.count("TOOLWORKING_COST_FOOD")) TOOLWORKING_COST_FOOD = Cast(std::get<0>(result["TOOLWORKING_COST_FOOD"])).ToI32();
    if (result.count("TOOLWORKING_COST_WOOD")) TOOLWORKING_COST_WOOD = Cast(std::get<0>(result["TOOLWORKING_COST_WOOD"])).ToI32();
    if (result.count("TOOLWORKING_COST_GOLD")) TOOLWORKING_COST_GOLD = Cast(std::get<0>(result["TOOLWORKING_COST_GOLD"])).ToI32();
    if (result.count("TOOLWORKING_COST_STONE")) TOOLWORKING_COST_STONE = Cast(std::get<0>(result["TOOLWORKING_COST_STONE"])).ToI32();
    if (result.count("TOOLWORKING_RESEARCH_TIME")) TOOLWORKING_RESEARCH_TIME = Cast(std::get<0>(result["TOOLWORKING_RESEARCH_TIME"])).ToI32();

    if (result.count("METALWORKING_COST_FOOD")) METALWORKING_COST_FOOD = Cast(std::get<0>(result["METALWORKING_COST_FOOD"])).ToI32();
    if (result.count("METALWORKING_COST_WOOD")) METALWORKING_COST_WOOD = Cast(std::get<0>(result["METALWORKING_COST_WOOD"])).ToI32();
    if (result.count("METALWORKING_COST_GOLD")) METALWORKING_COST_GOLD = Cast(std::get<0>(result["METALWORKING_COST_GOLD"])).ToI32();
    if (result.count("METALWORKING_COST_STONE")) METALWORKING_COST_STONE = Cast(std::get<0>(result["METALWORKING_COST_STONE"])).ToI32();
    if (result.count("METALWORKING_RESEARCH_TIME")) METALWORKING_RESEARCH_TIME = Cast(std::get<0>(result["METALWORKING_RESEARCH_TIME"])).ToI32();

    if (result.count("METALURGY_COST_FOOD")) METALURGY_COST_FOOD = Cast(std::get<0>(result["METALURGY_COST_FOOD"])).ToI32();
    if (result.count("METALURGY_COST_WOOD")) METALURGY_COST_WOOD = Cast(std::get<0>(result["METALURGY_COST_WOOD"])).ToI32();
    if (result.count("METALURGY_COST_GOLD")) METALURGY_COST_GOLD = Cast(std::get<0>(result["METALURGY_COST_GOLD"])).ToI32();
    if (result.count("METALURGY_COST_STONE")) METALURGY_COST_STONE = Cast(std::get<0>(result["METALURGY_COST_STONE"])).ToI32();
    if (result.count("METALURGY_RESEARCH_TIME")) METALURGY_RESEARCH_TIME = Cast(std::get<0>(result["METALURGY_RESEARCH_TIME"])).ToI32();

    if (result.count("BRONZE_SHIELD_COST_FOOD")) BRONZE_SHIELD_COST_FOOD = Cast(std::get<0>(result["BRONZE_SHIELD_COST_FOOD"])).ToI32();
    if (result.count("BRONZE_SHIELD_COST_WOOD")) BRONZE_SHIELD_COST_WOOD = Cast(std::get<0>(result["BRONZE_SHIELD_COST_WOOD"])).ToI32();
    if (result.count("BRONZE_SHIELD_COST_GOLD")) BRONZE_SHIELD_COST_GOLD = Cast(std::get<0>(result["BRONZE_SHIELD_COST_GOLD"])).ToI32();
    if (result.count("BRONZE_SHIELD_COST_STONE")) BRONZE_SHIELD_COST_STONE = Cast(std::get<0>(result["BRONZE_SHIELD_COST_STONE"])).ToI32();
    if (result.count("BRONZE_SHIELD_RESEARCH_TIME")) BRONZE_SHIELD_RESEARCH_TIME = Cast(std::get<0>(result["BRONZE_SHIELD_RESEARCH_TIME"])).ToI32();

    if (result.count("IRON_SHIELD_COST_FOOD")) IRON_SHIELD_COST_FOOD = Cast(std::get<0>(result["IRON_SHIELD_COST_FOOD"])).ToI32();
    if (result.count("IRON_SHIELD_COST_WOOD")) IRON_SHIELD_COST_WOOD = Cast(std::get<0>(result["IRON_SHIELD_COST_WOOD"])).ToI32();
    if (result.count("IRON_SHIELD_COST_GOLD")) IRON_SHIELD_COST_GOLD = Cast(std::get<0>(result["IRON_SHIELD_COST_GOLD"])).ToI32();
    if (result.count("IRON_SHIELD_COST_STONE")) IRON_SHIELD_COST_STONE = Cast(std::get<0>(result["IRON_SHIELD_COST_STONE"])).ToI32();
    if (result.count("IRON_SHIELD_RESEARCH_TIME")) IRON_SHIELD_RESEARCH_TIME = Cast(std::get<0>(result["IRON_SHIELD_RESEARCH_TIME"])).ToI32();

    if (result.count("TOWER_SHIELD_COST_FOOD")) TOWER_SHIELD_COST_FOOD = Cast(std::get<0>(result["TOWER_SHIELD_COST_FOOD"])).ToI32();
    if (result.count("TOWER_SHIELD_COST_WOOD")) TOWER_SHIELD_COST_WOOD = Cast(std::get<0>(result["TOWER_SHIELD_COST_WOOD"])).ToI32();
    if (result.count("TOWER_SHIELD_COST_GOLD")) TOWER_SHIELD_COST_GOLD = Cast(std::get<0>(result["TOWER_SHIELD_COST_GOLD"])).ToI32();
    if (result.count("TOWER_SHIELD_COST_STONE")) TOWER_SHIELD_COST_STONE = Cast(std::get<0>(result["TOWER_SHIELD_COST_STONE"])).ToI32();
    if (result.count("TOWER_SHIELD_RESEARCH_TIME")) TOWER_SHIELD_RESEARCH_TIME = Cast(std::get<0>(result["TOWER_SHIELD_RESEARCH_TIME"])).ToI32();

    // 时代
    if (result.count("TOOL_AGE_COST_FOOD")) TOOL_AGE_COST_FOOD = Cast(std::get<0>(result["TOOL_AGE_COST_FOOD"])).ToI32();
    if (result.count("TOOL_AGE_COST_WOOD")) TOOL_AGE_COST_WOOD = Cast(std::get<0>(result["TOOL_AGE_COST_WOOD"])).ToI32();
    if (result.count("TOOL_AGE_COST_GOLD")) TOOL_AGE_COST_GOLD = Cast(std::get<0>(result["TOOL_AGE_COST_GOLD"])).ToI32();
    if (result.count("TOOL_AGE_COST_STONE")) TOOL_AGE_COST_STONE = Cast(std::get<0>(result["TOOL_AGE_COST_STONE"])).ToI32();
    if (result.count("TOOL_AGE_RESEARCH_TIME")) TOOL_AGE_RESEARCH_TIME = Cast(std::get<0>(result["TOOL_AGE_RESEARCH_TIME"])).ToI32();

    if (result.count("BRONZE_AGE_COST_FOOD")) BRONZE_AGE_COST_FOOD = Cast(std::get<0>(result["BRONZE_AGE_COST_FOOD"])).ToI32();
    if (result.count("BRONZE_AGE_COST_WOOD")) BRONZE_AGE_COST_WOOD = Cast(std::get<0>(result["BRONZE_AGE_COST_WOOD"])).ToI32();
    if (result.count("BRONZE_AGE_COST_GOLD")) BRONZE_AGE_COST_GOLD = Cast(std::get<0>(result["BRONZE_AGE_COST_GOLD"])).ToI32();
    if (result.count("BRONZE_AGE_COST_STONE")) BRONZE_AGE_COST_STONE = Cast(std::get<0>(result["BRONZE_AGE_COST_STONE"])).ToI32();
    if (result.count("BRONZE_AGE_RESEARCH_TIME")) BRONZE_AGE_RESEARCH_TIME = Cast(std::get<0>(result["BRONZE_AGE_RESEARCH_TIME"])).ToI32();

    if (result.count("IRON_AGE_COST_FOOD")) IRON_AGE_COST_FOOD = Cast(std::get<0>(result["IRON_AGE_COST_FOOD"])).ToI32();
    if (result.count("IRON_AGE_COST_WOOD")) IRON_AGE_COST_WOOD = Cast(std::get<0>(result["IRON_AGE_COST_WOOD"])).ToI32();
    if (result.count("IRON_AGE_COST_GOLD")) IRON_AGE_COST_GOLD = Cast(std::get<0>(result["IRON_AGE_COST_GOLD"])).ToI32();
    if (result.count("IRON_AGE_COST_STONE")) IRON_AGE_COST_STONE = Cast(std::get<0>(result["IRON_AGE_COST_STONE"])).ToI32();
    if (result.count("IRON_AGE_RESEARCH_TIME")) IRON_AGE_RESEARCH_TIME = Cast(std::get<0>(result["IRON_AGE_RESEARCH_TIME"])).ToI32();

    // 建筑
    if (result.count("TOWN_CENTER_COST_FOOD")) TOWN_CENTER_COST_FOOD = Cast(std::get<0>(result["TOWN_CENTER_COST_FOOD"])).ToI32();
    if (result.count("TOWN_CENTER_COST_WOOD")) TOWN_CENTER_COST_WOOD = Cast(std::get<0>(result["TOWN_CENTER_COST_WOOD"])).ToI32();
    if (result.count("TOWN_CENTER_COST_GOLD")) TOWN_CENTER_COST_GOLD = Cast(std::get<0>(result["TOWN_CENTER_COST_GOLD"])).ToI32();
    if (result.count("TOWN_CENTER_COST_STONE")) TOWN_CENTER_COST_STONE = Cast(std::get<0>(result["TOWN_CENTER_COST_STONE"])).ToI32();
    if (result.count("TOWN_CENTER_BUILD_TIME")) TOWN_CENTER_BUILD_TIME = Cast(std::get<0>(result["TOWN_CENTER_BUILD_TIME"])).ToI32();
    if (result.count("TOWN_CENTER_HP")) TOWN_CENTER_HP = Cast(std::get<0>(result["TOWN_CENTER_HP"])).ToI32();

    if (result.count("HOUSE_COST_FOOD")) HOUSE_COST_FOOD = Cast(std::get<0>(result["HOUSE_COST_FOOD"])).ToI32();
    if (result.count("HOUSE_COST_WOOD")) HOUSE_COST_WOOD = Cast(std::get<0>(result["HOUSE_COST_WOOD"])).ToI32();
    if (result.count("HOUSE_COST_GOLD")) HOUSE_COST_GOLD = Cast(std::get<0>(result["HOUSE_COST_GOLD"])).ToI32();
    if (result.count("HOUSE_COST_STONE")) HOUSE_COST_STONE = Cast(std::get<0>(result["HOUSE_COST_STONE"])).ToI32();
    if (result.count("HOUSE_BUILD_TIME")) HOUSE_BUILD_TIME = Cast(std::get<0>(result["HOUSE_BUILD_TIME"])).ToI32();
    if (result.count("HOUSE_HP")) HOUSE_HP = Cast(std::get<0>(result["HOUSE_HP"])).ToI32();

    if (result.count("BARRACKS_COST_FOOD")) BARRACKS_COST_FOOD = Cast(std::get<0>(result["BARRACKS_COST_FOOD"])).ToI32();
    if (result.count("BARRACKS_COST_WOOD")) BARRACKS_COST_WOOD = Cast(std::get<0>(result["BARRACKS_COST_WOOD"])).ToI32();
    if (result.count("BARRACKS_COST_GOLD")) BARRACKS_COST_GOLD = Cast(std::get<0>(result["BARRACKS_COST_GOLD"])).ToI32();
    if (result.count("BARRACKS_COST_STONE")) BARRACKS_COST_STONE = Cast(std::get<0>(result["BARRACKS_COST_STONE"])).ToI32();
    if (result.count("BARRACKS_BUILD_TIME")) BARRACKS_BUILD_TIME = Cast(std::get<0>(result["BARRACKS_BUILD_TIME"])).ToI32();
    if (result.count("BARRACKS_HP")) BARRACKS_HP = Cast(std::get<0>(result["BARRACKS_HP"])).ToI32();

        // 步兵
    if (result.count("CLUBMAN_COST_FOOD")) CLUBMAN_COST_FOOD = Cast(std::get<0>(result["CLUBMAN_COST_FOOD"])).ToI32();
    if (result.count("CLUBMAN_COST_WOOD")) CLUBMAN_COST_WOOD = Cast(std::get<0>(result["CLUBMAN_COST_WOOD"])).ToI32();
    if (result.count("CLUBMAN_COST_GOLD")) CLUBMAN_COST_GOLD = Cast(std::get<0>(result["CLUBMAN_COST_GOLD"])).ToI32();
    if (result.count("CLUBMAN_COST_STONE")) CLUBMAN_COST_STONE = Cast(std::get<0>(result["CLUBMAN_COST_STONE"])).ToI32();
    if (result.count("CLUBMAN_TRAIN_TIME")) CLUBMAN_TRAIN_TIME = Cast(std::get<0>(result["CLUBMAN_TRAIN_TIME"])).ToI32();
    if (result.count("CLUBMAN_HP")) CLUBMAN_HP = Cast(std::get<0>(result["CLUBMAN_HP"])).ToI32();
    if (result.count("CLUBMAN_SPEED")) CLUBMAN_SPEED = Double(std::get<0>(result["CLUBMAN_SPEED"]).c_str());
    if (result.count("CLUBMAN_LINE_OF_SIGHT")) CLUBMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["CLUBMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CLUBMAN_HAND_ARMOR")) CLUBMAN_HAND_ARMOR = Cast(std::get<0>(result["CLUBMAN_HAND_ARMOR"])).ToI32();
    if (result.count("CLUBMAN_PIERCE_ARMOR")) CLUBMAN_PIERCE_ARMOR = Cast(std::get<0>(result["CLUBMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("CLUBMAN_ATTACK_INTERVAL")) CLUBMAN_ATTACK_INTERVAL = Double(std::get<0>(result["CLUBMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("CLUBMAN_ATTACK")) CLUBMAN_ATTACK = Cast(std::get<0>(result["CLUBMAN_ATTACK"])).ToI32();

    if (result.count("AXEMAN_COST_FOOD")) AXEMAN_COST_FOOD = Cast(std::get<0>(result["AXEMAN_COST_FOOD"])).ToI32();
    if (result.count("AXEMAN_COST_WOOD")) AXEMAN_COST_WOOD = Cast(std::get<0>(result["AXEMAN_COST_WOOD"])).ToI32();
    if (result.count("AXEMAN_COST_GOLD")) AXEMAN_COST_GOLD = Cast(std::get<0>(result["AXEMAN_COST_GOLD"])).ToI32();
    if (result.count("AXEMAN_COST_STONE")) AXEMAN_COST_STONE = Cast(std::get<0>(result["AXEMAN_COST_STONE"])).ToI32();
    if (result.count("AXEMAN_TRAIN_TIME")) AXEMAN_TRAIN_TIME = Cast(std::get<0>(result["AXEMAN_TRAIN_TIME"])).ToI32();
    if (result.count("AXEMAN_HP")) AXEMAN_HP = Cast(std::get<0>(result["AXEMAN_HP"])).ToI32();
    if (result.count("AXEMAN_SPEED")) AXEMAN_SPEED = Double(std::get<0>(result["AXEMAN_SPEED"]).c_str());
    if (result.count("AXEMAN_LINE_OF_SIGHT")) AXEMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["AXEMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("AXEMAN_HAND_ARMOR")) AXEMAN_HAND_ARMOR = Cast(std::get<0>(result["AXEMAN_HAND_ARMOR"])).ToI32();
    if (result.count("AXEMAN_PIERCE_ARMOR")) AXEMAN_PIERCE_ARMOR = Cast(std::get<0>(result["AXEMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("AXEMAN_ATTACK_INTERVAL")) AXEMAN_ATTACK_INTERVAL = Double(std::get<0>(result["AXEMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("AXEMAN_ATTACK")) AXEMAN_ATTACK = Cast(std::get<0>(result["AXEMAN_ATTACK"])).ToI32();

    if (result.count("SLINGER_COST_FOOD")) SLINGER_COST_FOOD = Cast(std::get<0>(result["SLINGER_COST_FOOD"])).ToI32();
    if (result.count("SLINGER_COST_WOOD")) SLINGER_COST_WOOD = Cast(std::get<0>(result["SLINGER_COST_WOOD"])).ToI32();
    if (result.count("SLINGER_COST_GOLD")) SLINGER_COST_GOLD = Cast(std::get<0>(result["SLINGER_COST_GOLD"])).ToI32();
    if (result.count("SLINGER_COST_STONE")) SLINGER_COST_STONE = Cast(std::get<0>(result["SLINGER_COST_STONE"])).ToI32();
    if (result.count("SLINGER_TRAIN_TIME")) SLINGER_TRAIN_TIME = Cast(std::get<0>(result["SLINGER_TRAIN_TIME"])).ToI32();
    if (result.count("SLINGER_HP")) SLINGER_HP = Cast(std::get<0>(result["SLINGER_HP"])).ToI32();
    if (result.count("SLINGER_SPEED")) SLINGER_SPEED = Double(std::get<0>(result["SLINGER_SPEED"]).c_str());
    if (result.count("SLINGER_LINE_OF_SIGHT")) SLINGER_LINE_OF_SIGHT = Cast(std::get<0>(result["SLINGER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("SLINGER_HAND_ARMOR")) SLINGER_HAND_ARMOR = Cast(std::get<0>(result["SLINGER_HAND_ARMOR"])).ToI32();
    if (result.count("SLINGER_PIERCE_ARMOR")) SLINGER_PIERCE_ARMOR = Cast(std::get<0>(result["SLINGER_PIERCE_ARMOR"])).ToI32();
    if (result.count("SLINGER_RANGE")) SLINGER_RANGE = Cast(std::get<0>(result["SLINGER_RANGE"])).ToI32();
    if (result.count("SLINGER_ATTACK_INTERVAL")) SLINGER_ATTACK_INTERVAL = Double(std::get<0>(result["SLINGER_ATTACK_INTERVAL"]).c_str());
    if (result.count("SLINGER_ATTACK")) SLINGER_ATTACK = Cast(std::get<0>(result["SLINGER_ATTACK"])).ToI32();

    if (result.count("SHORT_SWORDSMAN_COST_FOOD")) SHORT_SWORDSMAN_COST_FOOD = Cast(std::get<0>(result["SHORT_SWORDSMAN_COST_FOOD"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_COST_WOOD")) SHORT_SWORDSMAN_COST_WOOD = Cast(std::get<0>(result["SHORT_SWORDSMAN_COST_WOOD"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_COST_GOLD")) SHORT_SWORDSMAN_COST_GOLD = Cast(std::get<0>(result["SHORT_SWORDSMAN_COST_GOLD"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_COST_STONE")) SHORT_SWORDSMAN_COST_STONE = Cast(std::get<0>(result["SHORT_SWORDSMAN_COST_STONE"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_TRAIN_TIME")) SHORT_SWORDSMAN_TRAIN_TIME = Cast(std::get<0>(result["SHORT_SWORDSMAN_TRAIN_TIME"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_HP")) SHORT_SWORDSMAN_HP = Cast(std::get<0>(result["SHORT_SWORDSMAN_HP"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_SPEED")) SHORT_SWORDSMAN_SPEED = Double(std::get<0>(result["SHORT_SWORDSMAN_SPEED"]).c_str());
    if (result.count("SHORT_SWORDSMAN_LINE_OF_SIGHT")) SHORT_SWORDSMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["SHORT_SWORDSMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_HAND_ARMOR")) SHORT_SWORDSMAN_HAND_ARMOR = Cast(std::get<0>(result["SHORT_SWORDSMAN_HAND_ARMOR"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_PIERCE_ARMOR")) SHORT_SWORDSMAN_PIERCE_ARMOR = Cast(std::get<0>(result["SHORT_SWORDSMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("SHORT_SWORDSMAN_ATTACK_INTERVAL")) SHORT_SWORDSMAN_ATTACK_INTERVAL = Double(std::get<0>(result["SHORT_SWORDSMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("SHORT_SWORDSMAN_ATTACK")) SHORT_SWORDSMAN_ATTACK = Cast(std::get<0>(result["SHORT_SWORDSMAN_ATTACK"])).ToI32();

    if (result.count("BROAD_SWORDSMAN_COST_FOOD")) BROAD_SWORDSMAN_COST_FOOD = Cast(std::get<0>(result["BROAD_SWORDSMAN_COST_FOOD"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_COST_WOOD")) BROAD_SWORDSMAN_COST_WOOD = Cast(std::get<0>(result["BROAD_SWORDSMAN_COST_WOOD"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_COST_GOLD")) BROAD_SWORDSMAN_COST_GOLD = Cast(std::get<0>(result["BROAD_SWORDSMAN_COST_GOLD"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_COST_STONE")) BROAD_SWORDSMAN_COST_STONE = Cast(std::get<0>(result["BROAD_SWORDSMAN_COST_STONE"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_TRAIN_TIME")) BROAD_SWORDSMAN_TRAIN_TIME = Cast(std::get<0>(result["BROAD_SWORDSMAN_TRAIN_TIME"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_HP")) BROAD_SWORDSMAN_HP = Cast(std::get<0>(result["BROAD_SWORDSMAN_HP"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_SPEED")) BROAD_SWORDSMAN_SPEED = Double(std::get<0>(result["BROAD_SWORDSMAN_SPEED"]).c_str());
    if (result.count("BROAD_SWORDSMAN_LINE_OF_SIGHT")) BROAD_SWORDSMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["BROAD_SWORDSMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_HAND_ARMOR")) BROAD_SWORDSMAN_HAND_ARMOR = Cast(std::get<0>(result["BROAD_SWORDSMAN_HAND_ARMOR"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_PIERCE_ARMOR")) BROAD_SWORDSMAN_PIERCE_ARMOR = Cast(std::get<0>(result["BROAD_SWORDSMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("BROAD_SWORDSMAN_ATTACK_INTERVAL")) BROAD_SWORDSMAN_ATTACK_INTERVAL = Double(std::get<0>(result["BROAD_SWORDSMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("BROAD_SWORDSMAN_ATTACK")) BROAD_SWORDSMAN_ATTACK = Cast(std::get<0>(result["BROAD_SWORDSMAN_ATTACK"])).ToI32();

    if (result.count("LONG_SWORDSMAN_COST_FOOD")) LONG_SWORDSMAN_COST_FOOD = Cast(std::get<0>(result["LONG_SWORDSMAN_COST_FOOD"])).ToI32();
    if (result.count("LONG_SWORDSMAN_COST_WOOD")) LONG_SWORDSMAN_COST_WOOD = Cast(std::get<0>(result["LONG_SWORDSMAN_COST_WOOD"])).ToI32();
    if (result.count("LONG_SWORDSMAN_COST_GOLD")) LONG_SWORDSMAN_COST_GOLD = Cast(std::get<0>(result["LONG_SWORDSMAN_COST_GOLD"])).ToI32();
    if (result.count("LONG_SWORDSMAN_COST_STONE")) LONG_SWORDSMAN_COST_STONE = Cast(std::get<0>(result["LONG_SWORDSMAN_COST_STONE"])).ToI32();
    if (result.count("LONG_SWORDSMAN_TRAIN_TIME")) LONG_SWORDSMAN_TRAIN_TIME = Cast(std::get<0>(result["LONG_SWORDSMAN_TRAIN_TIME"])).ToI32();
    if (result.count("LONG_SWORDSMAN_HP")) LONG_SWORDSMAN_HP = Cast(std::get<0>(result["LONG_SWORDSMAN_HP"])).ToI32();
    if (result.count("LONG_SWORDSMAN_SPEED")) LONG_SWORDSMAN_SPEED = Double(std::get<0>(result["LONG_SWORDSMAN_SPEED"]).c_str());
    if (result.count("LONG_SWORDSMAN_LINE_OF_SIGHT")) LONG_SWORDSMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["LONG_SWORDSMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("LONG_SWORDSMAN_HAND_ARMOR")) LONG_SWORDSMAN_HAND_ARMOR = Cast(std::get<0>(result["LONG_SWORDSMAN_HAND_ARMOR"])).ToI32();
    if (result.count("LONG_SWORDSMAN_PIERCE_ARMOR")) LONG_SWORDSMAN_PIERCE_ARMOR = Cast(std::get<0>(result["LONG_SWORDSMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("LONG_SWORDSMAN_ATTACK_INTERVAL")) LONG_SWORDSMAN_ATTACK_INTERVAL = Double(std::get<0>(result["LONG_SWORDSMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("LONG_SWORDSMAN_ATTACK")) LONG_SWORDSMAN_ATTACK = Cast(std::get<0>(result["LONG_SWORDSMAN_ATTACK"])).ToI32();

    if (result.count("LEGION_COST_FOOD")) LEGION_COST_FOOD = Cast(std::get<0>(result["LEGION_COST_FOOD"])).ToI32();
    if (result.count("LEGION_COST_WOOD")) LEGION_COST_WOOD = Cast(std::get<0>(result["LEGION_COST_WOOD"])).ToI32();
    if (result.count("LEGION_COST_GOLD")) LEGION_COST_GOLD = Cast(std::get<0>(result["LEGION_COST_GOLD"])).ToI32();
    if (result.count("LEGION_COST_STONE")) LEGION_COST_STONE = Cast(std::get<0>(result["LEGION_COST_STONE"])).ToI32();
    if (result.count("LEGION_TRAIN_TIME")) LEGION_TRAIN_TIME = Cast(std::get<0>(result["LEGION_TRAIN_TIME"])).ToI32();
    if (result.count("LEGION_HP")) LEGION_HP = Cast(std::get<0>(result["LEGION_HP"])).ToI32();
    if (result.count("LEGION_SPEED")) LEGION_SPEED = Double(std::get<0>(result["LEGION_SPEED"]).c_str());
    if (result.count("LEGION_LINE_OF_SIGHT")) LEGION_LINE_OF_SIGHT = Cast(std::get<0>(result["LEGION_LINE_OF_SIGHT"])).ToI32();
    if (result.count("LEGION_HAND_ARMOR")) LEGION_HAND_ARMOR = Cast(std::get<0>(result["LEGION_HAND_ARMOR"])).ToI32();
    if (result.count("LEGION_PIERCE_ARMOR")) LEGION_PIERCE_ARMOR = Cast(std::get<0>(result["LEGION_PIERCE_ARMOR"])).ToI32();
    if (result.count("LEGION_ATTACK_INTERVAL")) LEGION_ATTACK_INTERVAL = Double(std::get<0>(result["LEGION_ATTACK_INTERVAL"]).c_str());
    if (result.count("LEGION_ATTACK")) LEGION_ATTACK = Cast(std::get<0>(result["LEGION_ATTACK"])).ToI32();

    if (result.count("HOPLITE_COST_FOOD")) HOPLITE_COST_FOOD = Cast(std::get<0>(result["HOPLITE_COST_FOOD"])).ToI32();
    if (result.count("HOPLITE_COST_WOOD")) HOPLITE_COST_WOOD = Cast(std::get<0>(result["HOPLITE_COST_WOOD"])).ToI32();
    if (result.count("HOPLITE_COST_GOLD")) HOPLITE_COST_GOLD = Cast(std::get<0>(result["HOPLITE_COST_GOLD"])).ToI32();
    if (result.count("HOPLITE_COST_STONE")) HOPLITE_COST_STONE = Cast(std::get<0>(result["HOPLITE_COST_STONE"])).ToI32();
    if (result.count("HOPLITE_TRAIN_TIME")) HOPLITE_TRAIN_TIME = Cast(std::get<0>(result["HOPLITE_TRAIN_TIME"])).ToI32();
    if (result.count("HOPLITE_HP")) HOPLITE_HP = Cast(std::get<0>(result["HOPLITE_HP"])).ToI32();
    if (result.count("HOPLITE_SPEED")) HOPLITE_SPEED = Double(std::get<0>(result["HOPLITE_SPEED"]).c_str());
    if (result.count("HOPLITE_LINE_OF_SIGHT")) HOPLITE_LINE_OF_SIGHT = Cast(std::get<0>(result["HOPLITE_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HOPLITE_HAND_ARMOR")) HOPLITE_HAND_ARMOR = Cast(std::get<0>(result["HOPLITE_HAND_ARMOR"])).ToI32();
    if (result.count("HOPLITE_PIERCE_ARMOR")) HOPLITE_PIERCE_ARMOR = Cast(std::get<0>(result["HOPLITE_PIERCE_ARMOR"])).ToI32();
    if (result.count("HOPLITE_ATTACK_INTERVAL")) HOPLITE_ATTACK_INTERVAL = Double(std::get<0>(result["HOPLITE_ATTACK_INTERVAL"]).c_str());
    if (result.count("HOPLITE_ATTACK")) HOPLITE_ATTACK = Cast(std::get<0>(result["HOPLITE_ATTACK"])).ToI32();

    if (result.count("PHALANX_COST_FOOD")) PHALANX_COST_FOOD = Cast(std::get<0>(result["PHALANX_COST_FOOD"])).ToI32();
    if (result.count("PHALANX_COST_WOOD")) PHALANX_COST_WOOD = Cast(std::get<0>(result["PHALANX_COST_WOOD"])).ToI32();
    if (result.count("PHALANX_COST_GOLD")) PHALANX_COST_GOLD = Cast(std::get<0>(result["PHALANX_COST_GOLD"])).ToI32();
    if (result.count("PHALANX_COST_STONE")) PHALANX_COST_STONE = Cast(std::get<0>(result["PHALANX_COST_STONE"])).ToI32();
    if (result.count("PHALANX_TRAIN_TIME")) PHALANX_TRAIN_TIME = Cast(std::get<0>(result["PHALANX_TRAIN_TIME"])).ToI32();
    if (result.count("PHALANX_HP")) PHALANX_HP = Cast(std::get<0>(result["PHALANX_HP"])).ToI32();
    if (result.count("PHALANX_SPEED")) PHALANX_SPEED = Double(std::get<0>(result["PHALANX_SPEED"]).c_str());
    if (result.count("PHALANX_LINE_OF_SIGHT")) PHALANX_LINE_OF_SIGHT = Cast(std::get<0>(result["PHALANX_LINE_OF_SIGHT"])).ToI32();
    if (result.count("PHALANX_HAND_ARMOR")) PHALANX_HAND_ARMOR = Cast(std::get<0>(result["PHALANX_HAND_ARMOR"])).ToI32();
    if (result.count("PHALANX_PIERCE_ARMOR")) PHALANX_PIERCE_ARMOR = Cast(std::get<0>(result["PHALANX_PIERCE_ARMOR"])).ToI32();
    if (result.count("PHALANX_ATTACK_INTERVAL")) PHALANX_ATTACK_INTERVAL = Double(std::get<0>(result["PHALANX_ATTACK_INTERVAL"]).c_str());
    if (result.count("PHALANX_ATTACK")) PHALANX_ATTACK = Cast(std::get<0>(result["PHALANX_ATTACK"])).ToI32();

    if (result.count("CENTURION_COST_FOOD")) CENTURION_COST_FOOD = Cast(std::get<0>(result["CENTURION_COST_FOOD"])).ToI32();
    if (result.count("CENTURION_COST_WOOD")) CENTURION_COST_WOOD = Cast(std::get<0>(result["CENTURION_COST_WOOD"])).ToI32();
    if (result.count("CENTURION_COST_GOLD")) CENTURION_COST_GOLD = Cast(std::get<0>(result["CENTURION_COST_GOLD"])).ToI32();
    if (result.count("CENTURION_COST_STONE")) CENTURION_COST_STONE = Cast(std::get<0>(result["CENTURION_COST_STONE"])).ToI32();
    if (result.count("CENTURION_TRAIN_TIME")) CENTURION_TRAIN_TIME = Cast(std::get<0>(result["CENTURION_TRAIN_TIME"])).ToI32();
    if (result.count("CENTURION_HP")) CENTURION_HP = Cast(std::get<0>(result["CENTURION_HP"])).ToI32();
    if (result.count("CENTURION_SPEED")) CENTURION_SPEED = Double(std::get<0>(result["CENTURION_SPEED"]).c_str());
    if (result.count("CENTURION_LINE_OF_SIGHT")) CENTURION_LINE_OF_SIGHT = Cast(std::get<0>(result["CENTURION_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CENTURION_HAND_ARMOR")) CENTURION_HAND_ARMOR = Cast(std::get<0>(result["CENTURION_HAND_ARMOR"])).ToI32();
    if (result.count("CENTURION_PIERCE_ARMOR")) CENTURION_PIERCE_ARMOR = Cast(std::get<0>(result["CENTURION_PIERCE_ARMOR"])).ToI32();
    if (result.count("CENTURION_ATTACK_INTERVAL")) CENTURION_ATTACK_INTERVAL = Double(std::get<0>(result["CENTURION_ATTACK_INTERVAL"]).c_str());
    if (result.count("CENTURION_ATTACK")) CENTURION_ATTACK = Cast(std::get<0>(result["CENTURION_ATTACK"])).ToI32();

    // 弓兵
    if (result.count("BOWMAN_COST_FOOD")) BOWMAN_COST_FOOD = Cast(std::get<0>(result["BOWMAN_COST_FOOD"])).ToI32();
    if (result.count("BOWMAN_COST_WOOD")) BOWMAN_COST_WOOD = Cast(std::get<0>(result["BOWMAN_COST_WOOD"])).ToI32();
    if (result.count("BOWMAN_COST_GOLD")) BOWMAN_COST_GOLD = Cast(std::get<0>(result["BOWMAN_COST_GOLD"])).ToI32();
    if (result.count("BOWMAN_COST_STONE")) BOWMAN_COST_STONE = Cast(std::get<0>(result["BOWMAN_COST_STONE"])).ToI32();
    if (result.count("BOWMAN_TRAIN_TIME")) BOWMAN_TRAIN_TIME = Cast(std::get<0>(result["BOWMAN_TRAIN_TIME"])).ToI32();
    if (result.count("BOWMAN_HP")) BOWMAN_HP = Cast(std::get<0>(result["BOWMAN_HP"])).ToI32();
    if (result.count("BOWMAN_SPEED")) BOWMAN_SPEED = Double(std::get<0>(result["BOWMAN_SPEED"]).c_str());
    if (result.count("BOWMAN_LINE_OF_SIGHT")) BOWMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["BOWMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("BOWMAN_HAND_ARMOR")) BOWMAN_HAND_ARMOR = Cast(std::get<0>(result["BOWMAN_HAND_ARMOR"])).ToI32();
    if (result.count("BOWMAN_PIERCE_ARMOR")) BOWMAN_PIERCE_ARMOR = Cast(std::get<0>(result["BOWMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("BOWMAN_RANGE")) BOWMAN_RANGE = Cast(std::get<0>(result["BOWMAN_RANGE"])).ToI32();
    if (result.count("BOWMAN_ATTACK_INTERVAL")) BOWMAN_ATTACK_INTERVAL = Double(std::get<0>(result["BOWMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("BOWMAN_ATTACK")) BOWMAN_ATTACK = Cast(std::get<0>(result["BOWMAN_ATTACK"])).ToI32();

    if (result.count("IMPROVED_BOWMAN_COST_FOOD")) IMPROVED_BOWMAN_COST_FOOD = Cast(std::get<0>(result["IMPROVED_BOWMAN_COST_FOOD"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_COST_WOOD")) IMPROVED_BOWMAN_COST_WOOD = Cast(std::get<0>(result["IMPROVED_BOWMAN_COST_WOOD"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_COST_GOLD")) IMPROVED_BOWMAN_COST_GOLD = Cast(std::get<0>(result["IMPROVED_BOWMAN_COST_GOLD"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_COST_STONE")) IMPROVED_BOWMAN_COST_STONE = Cast(std::get<0>(result["IMPROVED_BOWMAN_COST_STONE"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_TRAIN_TIME")) IMPROVED_BOWMAN_TRAIN_TIME = Cast(std::get<0>(result["IMPROVED_BOWMAN_TRAIN_TIME"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_HP")) IMPROVED_BOWMAN_HP = Cast(std::get<0>(result["IMPROVED_BOWMAN_HP"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_SPEED")) IMPROVED_BOWMAN_SPEED = Double(std::get<0>(result["IMPROVED_BOWMAN_SPEED"]).c_str());
    if (result.count("IMPROVED_BOWMAN_LINE_OF_SIGHT")) IMPROVED_BOWMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["IMPROVED_BOWMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_HAND_ARMOR")) IMPROVED_BOWMAN_HAND_ARMOR = Cast(std::get<0>(result["IMPROVED_BOWMAN_HAND_ARMOR"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_PIERCE_ARMOR")) IMPROVED_BOWMAN_PIERCE_ARMOR = Cast(std::get<0>(result["IMPROVED_BOWMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_RANGE")) IMPROVED_BOWMAN_RANGE = Cast(std::get<0>(result["IMPROVED_BOWMAN_RANGE"])).ToI32();
    if (result.count("IMPROVED_BOWMAN_ATTACK_INTERVAL")) IMPROVED_BOWMAN_ATTACK_INTERVAL = Double(std::get<0>(result["IMPROVED_BOWMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("IMPROVED_BOWMAN_ATTACK")) IMPROVED_BOWMAN_ATTACK = Cast(std::get<0>(result["IMPROVED_BOWMAN_ATTACK"])).ToI32();

    if (result.count("COMPOSITE_BOWMAN_COST_FOOD")) COMPOSITE_BOWMAN_COST_FOOD = Cast(std::get<0>(result["COMPOSITE_BOWMAN_COST_FOOD"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_COST_WOOD")) COMPOSITE_BOWMAN_COST_WOOD = Cast(std::get<0>(result["COMPOSITE_BOWMAN_COST_WOOD"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_COST_GOLD")) COMPOSITE_BOWMAN_COST_GOLD = Cast(std::get<0>(result["COMPOSITE_BOWMAN_COST_GOLD"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_COST_STONE")) COMPOSITE_BOWMAN_COST_STONE = Cast(std::get<0>(result["COMPOSITE_BOWMAN_COST_STONE"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_TRAIN_TIME")) COMPOSITE_BOWMAN_TRAIN_TIME = Cast(std::get<0>(result["COMPOSITE_BOWMAN_TRAIN_TIME"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_HP")) COMPOSITE_BOWMAN_HP = Cast(std::get<0>(result["COMPOSITE_BOWMAN_HP"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_SPEED")) COMPOSITE_BOWMAN_SPEED = Double(std::get<0>(result["COMPOSITE_BOWMAN_SPEED"]).c_str());
    if (result.count("COMPOSITE_BOWMAN_LINE_OF_SIGHT")) COMPOSITE_BOWMAN_LINE_OF_SIGHT = Cast(std::get<0>(result["COMPOSITE_BOWMAN_LINE_OF_SIGHT"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_HAND_ARMOR")) COMPOSITE_BOWMAN_HAND_ARMOR = Cast(std::get<0>(result["COMPOSITE_BOWMAN_HAND_ARMOR"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_PIERCE_ARMOR")) COMPOSITE_BOWMAN_PIERCE_ARMOR = Cast(std::get<0>(result["COMPOSITE_BOWMAN_PIERCE_ARMOR"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_RANGE")) COMPOSITE_BOWMAN_RANGE = Cast(std::get<0>(result["COMPOSITE_BOWMAN_RANGE"])).ToI32();
    if (result.count("COMPOSITE_BOWMAN_ATTACK_INTERVAL")) COMPOSITE_BOWMAN_ATTACK_INTERVAL = Double(std::get<0>(result["COMPOSITE_BOWMAN_ATTACK_INTERVAL"]).c_str());
    if (result.count("COMPOSITE_BOWMAN_ATTACK")) COMPOSITE_BOWMAN_ATTACK = Cast(std::get<0>(result["COMPOSITE_BOWMAN_ATTACK"])).ToI32();

    if (result.count("CHARIOT_ARCHER_COST_FOOD")) CHARIOT_ARCHER_COST_FOOD = Cast(std::get<0>(result["CHARIOT_ARCHER_COST_FOOD"])).ToI32();
    if (result.count("CHARIOT_ARCHER_COST_WOOD")) CHARIOT_ARCHER_COST_WOOD = Cast(std::get<0>(result["CHARIOT_ARCHER_COST_WOOD"])).ToI32();
    if (result.count("CHARIOT_ARCHER_COST_GOLD")) CHARIOT_ARCHER_COST_GOLD = Cast(std::get<0>(result["CHARIOT_ARCHER_COST_GOLD"])).ToI32();
    if (result.count("CHARIOT_ARCHER_COST_STONE")) CHARIOT_ARCHER_COST_STONE = Cast(std::get<0>(result["CHARIOT_ARCHER_COST_STONE"])).ToI32();
    if (result.count("CHARIOT_ARCHER_TRAIN_TIME")) CHARIOT_ARCHER_TRAIN_TIME = Cast(std::get<0>(result["CHARIOT_ARCHER_TRAIN_TIME"])).ToI32();
    if (result.count("CHARIOT_ARCHER_HP")) CHARIOT_ARCHER_HP = Cast(std::get<0>(result["CHARIOT_ARCHER_HP"])).ToI32();
    if (result.count("CHARIOT_ARCHER_SPEED")) CHARIOT_ARCHER_SPEED = Double(std::get<0>(result["CHARIOT_ARCHER_SPEED"]).c_str());
    if (result.count("CHARIOT_ARCHER_LINE_OF_SIGHT")) CHARIOT_ARCHER_LINE_OF_SIGHT = Cast(std::get<0>(result["CHARIOT_ARCHER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CHARIOT_ARCHER_HAND_ARMOR")) CHARIOT_ARCHER_HAND_ARMOR = Cast(std::get<0>(result["CHARIOT_ARCHER_HAND_ARMOR"])).ToI32();
    if (result.count("CHARIOT_ARCHER_PIERCE_ARMOR")) CHARIOT_ARCHER_PIERCE_ARMOR = Cast(std::get<0>(result["CHARIOT_ARCHER_PIERCE_ARMOR"])).ToI32();
    if (result.count("CHARIOT_ARCHER_RANGE")) CHARIOT_ARCHER_RANGE = Cast(std::get<0>(result["CHARIOT_ARCHER_RANGE"])).ToI32();
    if (result.count("CHARIOT_ARCHER_ATTACK_INTERVAL")) CHARIOT_ARCHER_ATTACK_INTERVAL = Double(std::get<0>(result["CHARIOT_ARCHER_ATTACK_INTERVAL"]).c_str());
    if (result.count("CHARIOT_ARCHER_ATTACK")) CHARIOT_ARCHER_ATTACK = Cast(std::get<0>(result["CHARIOT_ARCHER_ATTACK"])).ToI32();

    if (result.count("ELEPHANT_ARCHER_COST_FOOD")) ELEPHANT_ARCHER_COST_FOOD = Cast(std::get<0>(result["ELEPHANT_ARCHER_COST_FOOD"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_COST_WOOD")) ELEPHANT_ARCHER_COST_WOOD = Cast(std::get<0>(result["ELEPHANT_ARCHER_COST_WOOD"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_COST_GOLD")) ELEPHANT_ARCHER_COST_GOLD = Cast(std::get<0>(result["ELEPHANT_ARCHER_COST_GOLD"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_COST_STONE")) ELEPHANT_ARCHER_COST_STONE = Cast(std::get<0>(result["ELEPHANT_ARCHER_COST_STONE"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_TRAIN_TIME")) ELEPHANT_ARCHER_TRAIN_TIME = Cast(std::get<0>(result["ELEPHANT_ARCHER_TRAIN_TIME"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_HP")) ELEPHANT_ARCHER_HP = Cast(std::get<0>(result["ELEPHANT_ARCHER_HP"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_SPEED")) ELEPHANT_ARCHER_SPEED = Double(std::get<0>(result["ELEPHANT_ARCHER_SPEED"]).c_str());
    if (result.count("ELEPHANT_ARCHER_LINE_OF_SIGHT")) ELEPHANT_ARCHER_LINE_OF_SIGHT = Cast(std::get<0>(result["ELEPHANT_ARCHER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_HAND_ARMOR")) ELEPHANT_ARCHER_HAND_ARMOR = Cast(std::get<0>(result["ELEPHANT_ARCHER_HAND_ARMOR"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_PIERCE_ARMOR")) ELEPHANT_ARCHER_PIERCE_ARMOR = Cast(std::get<0>(result["ELEPHANT_ARCHER_PIERCE_ARMOR"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_RANGE")) ELEPHANT_ARCHER_RANGE = Cast(std::get<0>(result["ELEPHANT_ARCHER_RANGE"])).ToI32();
    if (result.count("ELEPHANT_ARCHER_ATTACK_INTERVAL")) ELEPHANT_ARCHER_ATTACK_INTERVAL = Double(std::get<0>(result["ELEPHANT_ARCHER_ATTACK_INTERVAL"]).c_str());
    if (result.count("ELEPHANT_ARCHER_ATTACK")) ELEPHANT_ARCHER_ATTACK = Cast(std::get<0>(result["ELEPHANT_ARCHER_ATTACK"])).ToI32();

    if (result.count("HORSE_ARCHER_COST_FOOD")) HORSE_ARCHER_COST_FOOD = Cast(std::get<0>(result["HORSE_ARCHER_COST_FOOD"])).ToI32();
    if (result.count("HORSE_ARCHER_COST_WOOD")) HORSE_ARCHER_COST_WOOD = Cast(std::get<0>(result["HORSE_ARCHER_COST_WOOD"])).ToI32();
    if (result.count("HORSE_ARCHER_COST_GOLD")) HORSE_ARCHER_COST_GOLD = Cast(std::get<0>(result["HORSE_ARCHER_COST_GOLD"])).ToI32();
    if (result.count("HORSE_ARCHER_COST_STONE")) HORSE_ARCHER_COST_STONE = Cast(std::get<0>(result["HORSE_ARCHER_COST_STONE"])).ToI32();
    if (result.count("HORSE_ARCHER_TRAIN_TIME")) HORSE_ARCHER_TRAIN_TIME = Cast(std::get<0>(result["HORSE_ARCHER_TRAIN_TIME"])).ToI32();
    if (result.count("HORSE_ARCHER_HP")) HORSE_ARCHER_HP = Cast(std::get<0>(result["HORSE_ARCHER_HP"])).ToI32();
    if (result.count("HORSE_ARCHER_SPEED")) HORSE_ARCHER_SPEED = Double(std::get<0>(result["HORSE_ARCHER_SPEED"]).c_str());
    if (result.count("HORSE_ARCHER_LINE_OF_SIGHT")) HORSE_ARCHER_LINE_OF_SIGHT = Cast(std::get<0>(result["HORSE_ARCHER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HORSE_ARCHER_HAND_ARMOR")) HORSE_ARCHER_HAND_ARMOR = Cast(std::get<0>(result["HORSE_ARCHER_HAND_ARMOR"])).ToI32();
    if (result.count("HORSE_ARCHER_PIERCE_ARMOR")) HORSE_ARCHER_PIERCE_ARMOR = Cast(std::get<0>(result["HORSE_ARCHER_PIERCE_ARMOR"])).ToI32();
    if (result.count("HORSE_ARCHER_RANGE")) HORSE_ARCHER_RANGE = Cast(std::get<0>(result["HORSE_ARCHER_RANGE"])).ToI32();
    if (result.count("HORSE_ARCHER_ATTACK_INTERVAL")) HORSE_ARCHER_ATTACK_INTERVAL = Double(std::get<0>(result["HORSE_ARCHER_ATTACK_INTERVAL"]).c_str());
    if (result.count("HORSE_ARCHER_ATTACK")) HORSE_ARCHER_ATTACK = Cast(std::get<0>(result["HORSE_ARCHER_ATTACK"])).ToI32();

    if (result.count("HEAVY_HORSE_ARCHER_COST_FOOD")) HEAVY_HORSE_ARCHER_COST_FOOD = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_COST_FOOD"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_COST_WOOD")) HEAVY_HORSE_ARCHER_COST_WOOD = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_COST_WOOD"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_COST_GOLD")) HEAVY_HORSE_ARCHER_COST_GOLD = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_COST_GOLD"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_COST_STONE")) HEAVY_HORSE_ARCHER_COST_STONE = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_COST_STONE"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_TRAIN_TIME")) HEAVY_HORSE_ARCHER_TRAIN_TIME = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_TRAIN_TIME"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_HP")) HEAVY_HORSE_ARCHER_HP = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_HP"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_SPEED")) HEAVY_HORSE_ARCHER_SPEED = Double(std::get<0>(result["HEAVY_HORSE_ARCHER_SPEED"]).c_str());
    if (result.count("HEAVY_HORSE_ARCHER_LINE_OF_SIGHT")) HEAVY_HORSE_ARCHER_LINE_OF_SIGHT = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_HAND_ARMOR")) HEAVY_HORSE_ARCHER_HAND_ARMOR = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_HAND_ARMOR"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_PIERCE_ARMOR")) HEAVY_HORSE_ARCHER_PIERCE_ARMOR = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_PIERCE_ARMOR"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_RANGE")) HEAVY_HORSE_ARCHER_RANGE = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_RANGE"])).ToI32();
    if (result.count("HEAVY_HORSE_ARCHER_ATTACK_INTERVAL")) HEAVY_HORSE_ARCHER_ATTACK_INTERVAL = Double(std::get<0>(result["HEAVY_HORSE_ARCHER_ATTACK_INTERVAL"]).c_str());
    if (result.count("HEAVY_HORSE_ARCHER_ATTACK")) HEAVY_HORSE_ARCHER_ATTACK = Cast(std::get<0>(result["HEAVY_HORSE_ARCHER_ATTACK"])).ToI32();
    
        // 骑兵
    if (result.count("SCOUT_COST_FOOD")) SCOUT_COST_FOOD = Cast(std::get<0>(result["SCOUT_COST_FOOD"])).ToI32();
    if (result.count("SCOUT_COST_WOOD")) SCOUT_COST_WOOD = Cast(std::get<0>(result["SCOUT_COST_WOOD"])).ToI32();
    if (result.count("SCOUT_COST_GOLD")) SCOUT_COST_GOLD = Cast(std::get<0>(result["SCOUT_COST_GOLD"])).ToI32();
    if (result.count("SCOUT_COST_STONE")) SCOUT_COST_STONE = Cast(std::get<0>(result["SCOUT_COST_STONE"])).ToI32();
    if (result.count("SCOUT_TRAIN_TIME")) SCOUT_TRAIN_TIME = Cast(std::get<0>(result["SCOUT_TRAIN_TIME"])).ToI32();
    if (result.count("SCOUT_HP")) SCOUT_HP = Cast(std::get<0>(result["SCOUT_HP"])).ToI32();
    if (result.count("SCOUT_SPEED")) SCOUT_SPEED = Double(std::get<0>(result["SCOUT_SPEED"]).c_str());
    if (result.count("SCOUT_LINE_OF_SIGHT")) SCOUT_LINE_OF_SIGHT = Cast(std::get<0>(result["SCOUT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("SCOUT_HAND_ARMOR")) SCOUT_HAND_ARMOR = Cast(std::get<0>(result["SCOUT_HAND_ARMOR"])).ToI32();
    if (result.count("SCOUT_PIERCE_ARMOR")) SCOUT_PIERCE_ARMOR = Cast(std::get<0>(result["SCOUT_PIERCE_ARMOR"])).ToI32();
    if (result.count("SCOUT_ATTACK_INTERVAL")) SCOUT_ATTACK_INTERVAL = Double(std::get<0>(result["SCOUT_ATTACK_INTERVAL"]).c_str());
    if (result.count("SCOUT_ATTACK")) SCOUT_ATTACK = Cast(std::get<0>(result["SCOUT_ATTACK"])).ToI32();

    if (result.count("CAMEL_RIDER_COST_FOOD")) CAMEL_RIDER_COST_FOOD = Cast(std::get<0>(result["CAMEL_RIDER_COST_FOOD"])).ToI32();
    if (result.count("CAMEL_RIDER_COST_WOOD")) CAMEL_RIDER_COST_WOOD = Cast(std::get<0>(result["CAMEL_RIDER_COST_WOOD"])).ToI32();
    if (result.count("CAMEL_RIDER_COST_GOLD")) CAMEL_RIDER_COST_GOLD = Cast(std::get<0>(result["CAMEL_RIDER_COST_GOLD"])).ToI32();
    if (result.count("CAMEL_RIDER_COST_STONE")) CAMEL_RIDER_COST_STONE = Cast(std::get<0>(result["CAMEL_RIDER_COST_STONE"])).ToI32();
    if (result.count("CAMEL_RIDER_TRAIN_TIME")) CAMEL_RIDER_TRAIN_TIME = Cast(std::get<0>(result["CAMEL_RIDER_TRAIN_TIME"])).ToI32();
    if (result.count("CAMEL_RIDER_HP")) CAMEL_RIDER_HP = Cast(std::get<0>(result["CAMEL_RIDER_HP"])).ToI32();
    if (result.count("CAMEL_RIDER_SPEED")) CAMEL_RIDER_SPEED = Double(std::get<0>(result["CAMEL_RIDER_SPEED"]).c_str());
    if (result.count("CAMEL_RIDER_LINE_OF_SIGHT")) CAMEL_RIDER_LINE_OF_SIGHT = Cast(std::get<0>(result["CAMEL_RIDER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CAMEL_RIDER_HAND_ARMOR")) CAMEL_RIDER_HAND_ARMOR = Cast(std::get<0>(result["CAMEL_RIDER_HAND_ARMOR"])).ToI32();
    if (result.count("CAMEL_RIDER_PIERCE_ARMOR")) CAMEL_RIDER_PIERCE_ARMOR = Cast(std::get<0>(result["CAMEL_RIDER_PIERCE_ARMOR"])).ToI32();
    if (result.count("CAMEL_RIDER_ATTACK_INTERVAL")) CAMEL_RIDER_ATTACK_INTERVAL = Double(std::get<0>(result["CAMEL_RIDER_ATTACK_INTERVAL"]).c_str());
    if (result.count("CAMEL_RIDER_ATTACK")) CAMEL_RIDER_ATTACK = Cast(std::get<0>(result["CAMEL_RIDER_ATTACK"])).ToI32();

    if (result.count("CHARIOT_COST_FOOD")) CHARIOT_COST_FOOD = Cast(std::get<0>(result["CHARIOT_COST_FOOD"])).ToI32();
    if (result.count("CHARIOT_COST_WOOD")) CHARIOT_COST_WOOD = Cast(std::get<0>(result["CHARIOT_COST_WOOD"])).ToI32();
    if (result.count("CHARIOT_COST_GOLD")) CHARIOT_COST_GOLD = Cast(std::get<0>(result["CHARIOT_COST_GOLD"])).ToI32();
    if (result.count("CHARIOT_COST_STONE")) CHARIOT_COST_STONE = Cast(std::get<0>(result["CHARIOT_COST_STONE"])).ToI32();
    if (result.count("CHARIOT_TRAIN_TIME")) CHARIOT_TRAIN_TIME = Cast(std::get<0>(result["CHARIOT_TRAIN_TIME"])).ToI32();
    if (result.count("CHARIOT_HP")) CHARIOT_HP = Cast(std::get<0>(result["CHARIOT_HP"])).ToI32();
    if (result.count("CHARIOT_SPEED")) CHARIOT_SPEED = Double(std::get<0>(result["CHARIOT_SPEED"]).c_str());
    if (result.count("CHARIOT_LINE_OF_SIGHT")) CHARIOT_LINE_OF_SIGHT = Cast(std::get<0>(result["CHARIOT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CHARIOT_HAND_ARMOR")) CHARIOT_HAND_ARMOR = Cast(std::get<0>(result["CHARIOT_HAND_ARMOR"])).ToI32();
    if (result.count("CHARIOT_PIERCE_ARMOR")) CHARIOT_PIERCE_ARMOR = Cast(std::get<0>(result["CHARIOT_PIERCE_ARMOR"])).ToI32();
    if (result.count("CHARIOT_ATTACK_INTERVAL")) CHARIOT_ATTACK_INTERVAL = Double(std::get<0>(result["CHARIOT_ATTACK_INTERVAL"]).c_str());
    if (result.count("CHARIOT_ATTACK")) CHARIOT_ATTACK = Cast(std::get<0>(result["CHARIOT_ATTACK"])).ToI32();

    if (result.count("SCYTHE_CHARIOT_COST_FOOD")) SCYTHE_CHARIOT_COST_FOOD = Cast(std::get<0>(result["SCYTHE_CHARIOT_COST_FOOD"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_COST_WOOD")) SCYTHE_CHARIOT_COST_WOOD = Cast(std::get<0>(result["SCYTHE_CHARIOT_COST_WOOD"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_COST_GOLD")) SCYTHE_CHARIOT_COST_GOLD = Cast(std::get<0>(result["SCYTHE_CHARIOT_COST_GOLD"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_COST_STONE")) SCYTHE_CHARIOT_COST_STONE = Cast(std::get<0>(result["SCYTHE_CHARIOT_COST_STONE"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_TRAIN_TIME")) SCYTHE_CHARIOT_TRAIN_TIME = Cast(std::get<0>(result["SCYTHE_CHARIOT_TRAIN_TIME"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_HP")) SCYTHE_CHARIOT_HP = Cast(std::get<0>(result["SCYTHE_CHARIOT_HP"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_SPEED")) SCYTHE_CHARIOT_SPEED = Double(std::get<0>(result["SCYTHE_CHARIOT_SPEED"]).c_str());
    if (result.count("SCYTHE_CHARIOT_LINE_OF_SIGHT")) SCYTHE_CHARIOT_LINE_OF_SIGHT = Cast(std::get<0>(result["SCYTHE_CHARIOT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_HAND_ARMOR")) SCYTHE_CHARIOT_HAND_ARMOR = Cast(std::get<0>(result["SCYTHE_CHARIOT_HAND_ARMOR"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_PIERCE_ARMOR")) SCYTHE_CHARIOT_PIERCE_ARMOR = Cast(std::get<0>(result["SCYTHE_CHARIOT_PIERCE_ARMOR"])).ToI32();
    if (result.count("SCYTHE_CHARIOT_ATTACK_INTERVAL")) SCYTHE_CHARIOT_ATTACK_INTERVAL = Double(std::get<0>(result["SCYTHE_CHARIOT_ATTACK_INTERVAL"]).c_str());
    if (result.count("SCYTHE_CHARIOT_ATTACK")) SCYTHE_CHARIOT_ATTACK = Cast(std::get<0>(result["SCYTHE_CHARIOT_ATTACK"])).ToI32();

    if (result.count("CAVALRY_COST_FOOD")) CAVALRY_COST_FOOD = Cast(std::get<0>(result["CAVALRY_COST_FOOD"])).ToI32();
    if (result.count("CAVALRY_COST_WOOD")) CAVALRY_COST_WOOD = Cast(std::get<0>(result["CAVALRY_COST_WOOD"])).ToI32();
    if (result.count("CAVALRY_COST_GOLD")) CAVALRY_COST_GOLD = Cast(std::get<0>(result["CAVALRY_COST_GOLD"])).ToI32();
    if (result.count("CAVALRY_COST_STONE")) CAVALRY_COST_STONE = Cast(std::get<0>(result["CAVALRY_COST_STONE"])).ToI32();
    if (result.count("CAVALRY_TRAIN_TIME")) CAVALRY_TRAIN_TIME = Cast(std::get<0>(result["CAVALRY_TRAIN_TIME"])).ToI32();
    if (result.count("CAVALRY_HP")) CAVALRY_HP = Cast(std::get<0>(result["CAVALRY_HP"])).ToI32();
    if (result.count("CAVALRY_SPEED")) CAVALRY_SPEED = Double(std::get<0>(result["CAVALRY_SPEED"]).c_str());
    if (result.count("CAVALRY_LINE_OF_SIGHT")) CAVALRY_LINE_OF_SIGHT = Cast(std::get<0>(result["CAVALRY_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CAVALRY_HAND_ARMOR")) CAVALRY_HAND_ARMOR = Cast(std::get<0>(result["CAVALRY_HAND_ARMOR"])).ToI32();
    if (result.count("CAVALRY_PIERCE_ARMOR")) CAVALRY_PIERCE_ARMOR = Cast(std::get<0>(result["CAVALRY_PIERCE_ARMOR"])).ToI32();
    if (result.count("CAVALRY_ATTACK_INTERVAL")) CAVALRY_ATTACK_INTERVAL = Double(std::get<0>(result["CAVALRY_ATTACK_INTERVAL"]).c_str());
    if (result.count("CAVALRY_ATTACK")) CAVALRY_ATTACK = Cast(std::get<0>(result["CAVALRY_ATTACK"])).ToI32();

    if (result.count("HEAVY_CAVALRY_COST_FOOD")) HEAVY_CAVALRY_COST_FOOD = Cast(std::get<0>(result["HEAVY_CAVALRY_COST_FOOD"])).ToI32();
    if (result.count("HEAVY_CAVALRY_COST_WOOD")) HEAVY_CAVALRY_COST_WOOD = Cast(std::get<0>(result["HEAVY_CAVALRY_COST_WOOD"])).ToI32();
    if (result.count("HEAVY_CAVALRY_COST_GOLD")) HEAVY_CAVALRY_COST_GOLD = Cast(std::get<0>(result["HEAVY_CAVALRY_COST_GOLD"])).ToI32();
    if (result.count("HEAVY_CAVALRY_COST_STONE")) HEAVY_CAVALRY_COST_STONE = Cast(std::get<0>(result["HEAVY_CAVALRY_COST_STONE"])).ToI32();
    if (result.count("HEAVY_CAVALRY_TRAIN_TIME")) HEAVY_CAVALRY_TRAIN_TIME = Cast(std::get<0>(result["HEAVY_CAVALRY_TRAIN_TIME"])).ToI32();
    if (result.count("HEAVY_CAVALRY_HP")) HEAVY_CAVALRY_HP = Cast(std::get<0>(result["HEAVY_CAVALRY_HP"])).ToI32();
    if (result.count("HEAVY_CAVALRY_SPEED")) HEAVY_CAVALRY_SPEED = Double(std::get<0>(result["HEAVY_CAVALRY_SPEED"]).c_str());
    if (result.count("HEAVY_CAVALRY_LINE_OF_SIGHT")) HEAVY_CAVALRY_LINE_OF_SIGHT = Cast(std::get<0>(result["HEAVY_CAVALRY_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HEAVY_CAVALRY_HAND_ARMOR")) HEAVY_CAVALRY_HAND_ARMOR = Cast(std::get<0>(result["HEAVY_CAVALRY_HAND_ARMOR"])).ToI32();
    if (result.count("HEAVY_CAVALRY_PIERCE_ARMOR")) HEAVY_CAVALRY_PIERCE_ARMOR = Cast(std::get<0>(result["HEAVY_CAVALRY_PIERCE_ARMOR"])).ToI32();
    if (result.count("HEAVY_CAVALRY_ATTACK_INTERVAL")) HEAVY_CAVALRY_ATTACK_INTERVAL = Double(std::get<0>(result["HEAVY_CAVALRY_ATTACK_INTERVAL"]).c_str());
    if (result.count("HEAVY_CAVALRY_ATTACK")) HEAVY_CAVALRY_ATTACK = Cast(std::get<0>(result["HEAVY_CAVALRY_ATTACK"])).ToI32();

    if (result.count("CATAPHRACT_COST_FOOD")) CATAPHRACT_COST_FOOD = Cast(std::get<0>(result["CATAPHRACT_COST_FOOD"])).ToI32();
    if (result.count("CATAPHRACT_COST_WOOD")) CATAPHRACT_COST_WOOD = Cast(std::get<0>(result["CATAPHRACT_COST_WOOD"])).ToI32();
    if (result.count("CATAPHRACT_COST_GOLD")) CATAPHRACT_COST_GOLD = Cast(std::get<0>(result["CATAPHRACT_COST_GOLD"])).ToI32();
    if (result.count("CATAPHRACT_COST_STONE")) CATAPHRACT_COST_STONE = Cast(std::get<0>(result["CATAPHRACT_COST_STONE"])).ToI32();
    if (result.count("CATAPHRACT_TRAIN_TIME")) CATAPHRACT_TRAIN_TIME = Cast(std::get<0>(result["CATAPHRACT_TRAIN_TIME"])).ToI32();
    if (result.count("CATAPHRACT_HP")) CATAPHRACT_HP = Cast(std::get<0>(result["CATAPHRACT_HP"])).ToI32();
    if (result.count("CATAPHRACT_SPEED")) CATAPHRACT_SPEED = Double(std::get<0>(result["CATAPHRACT_SPEED"]).c_str());
    if (result.count("CATAPHRACT_LINE_OF_SIGHT")) CATAPHRACT_LINE_OF_SIGHT = Cast(std::get<0>(result["CATAPHRACT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CATAPHRACT_HAND_ARMOR")) CATAPHRACT_HAND_ARMOR = Cast(std::get<0>(result["CATAPHRACT_HAND_ARMOR"])).ToI32();
    if (result.count("CATAPHRACT_PIERCE_ARMOR")) CATAPHRACT_PIERCE_ARMOR = Cast(std::get<0>(result["CATAPHRACT_PIERCE_ARMOR"])).ToI32();
    if (result.count("CATAPHRACT_ATTACK_INTERVAL")) CATAPHRACT_ATTACK_INTERVAL = Double(std::get<0>(result["CATAPHRACT_ATTACK_INTERVAL"]).c_str());
    if (result.count("CATAPHRACT_ATTACK")) CATAPHRACT_ATTACK = Cast(std::get<0>(result["CATAPHRACT_ATTACK"])).ToI32();

    // 战象
    if (result.count("WAR_ELEPHANT_COST_FOOD")) WAR_ELEPHANT_COST_FOOD = Cast(std::get<0>(result["WAR_ELEPHANT_COST_FOOD"])).ToI32();
    if (result.count("WAR_ELEPHANT_COST_WOOD")) WAR_ELEPHANT_COST_WOOD = Cast(std::get<0>(result["WAR_ELEPHANT_COST_WOOD"])).ToI32();
    if (result.count("WAR_ELEPHANT_COST_GOLD")) WAR_ELEPHANT_COST_GOLD = Cast(std::get<0>(result["WAR_ELEPHANT_COST_GOLD"])).ToI32();
    if (result.count("WAR_ELEPHANT_COST_STONE")) WAR_ELEPHANT_COST_STONE = Cast(std::get<0>(result["WAR_ELEPHANT_COST_STONE"])).ToI32();
    if (result.count("WAR_ELEPHANT_TRAIN_TIME")) WAR_ELEPHANT_TRAIN_TIME = Cast(std::get<0>(result["WAR_ELEPHANT_TRAIN_TIME"])).ToI32();
    if (result.count("WAR_ELEPHANT_HP")) WAR_ELEPHANT_HP = Cast(std::get<0>(result["WAR_ELEPHANT_HP"])).ToI32();
    if (result.count("WAR_ELEPHANT_SPEED")) WAR_ELEPHANT_SPEED = Double(std::get<0>(result["WAR_ELEPHANT_SPEED"]).c_str());
    if (result.count("WAR_ELEPHANT_LINE_OF_SIGHT")) WAR_ELEPHANT_LINE_OF_SIGHT = Cast(std::get<0>(result["WAR_ELEPHANT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("WAR_ELEPHANT_HAND_ARMOR")) WAR_ELEPHANT_HAND_ARMOR = Cast(std::get<0>(result["WAR_ELEPHANT_HAND_ARMOR"])).ToI32();
    if (result.count("WAR_ELEPHANT_PIERCE_ARMOR")) WAR_ELEPHANT_PIERCE_ARMOR = Cast(std::get<0>(result["WAR_ELEPHANT_PIERCE_ARMOR"])).ToI32();
    if (result.count("WAR_ELEPHANT_ATTACK_INTERVAL")) WAR_ELEPHANT_ATTACK_INTERVAL = Double(std::get<0>(result["WAR_ELEPHANT_ATTACK_INTERVAL"]).c_str());
    if (result.count("WAR_ELEPHANT_ATTACK")) WAR_ELEPHANT_ATTACK = Cast(std::get<0>(result["WAR_ELEPHANT_ATTACK"])).ToI32();

    if (result.count("ARMORED_ELEPHANT_COST_FOOD")) ARMORED_ELEPHANT_COST_FOOD = Cast(std::get<0>(result["ARMORED_ELEPHANT_COST_FOOD"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_COST_WOOD")) ARMORED_ELEPHANT_COST_WOOD = Cast(std::get<0>(result["ARMORED_ELEPHANT_COST_WOOD"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_COST_GOLD")) ARMORED_ELEPHANT_COST_GOLD = Cast(std::get<0>(result["ARMORED_ELEPHANT_COST_GOLD"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_COST_STONE")) ARMORED_ELEPHANT_COST_STONE = Cast(std::get<0>(result["ARMORED_ELEPHANT_COST_STONE"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_TRAIN_TIME")) ARMORED_ELEPHANT_TRAIN_TIME = Cast(std::get<0>(result["ARMORED_ELEPHANT_TRAIN_TIME"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_HP")) ARMORED_ELEPHANT_HP = Cast(std::get<0>(result["ARMORED_ELEPHANT_HP"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_SPEED")) ARMORED_ELEPHANT_SPEED = Double(std::get<0>(result["ARMORED_ELEPHANT_SPEED"]).c_str());
    if (result.count("ARMORED_ELEPHANT_LINE_OF_SIGHT")) ARMORED_ELEPHANT_LINE_OF_SIGHT = Cast(std::get<0>(result["ARMORED_ELEPHANT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_HAND_ARMOR")) ARMORED_ELEPHANT_HAND_ARMOR = Cast(std::get<0>(result["ARMORED_ELEPHANT_HAND_ARMOR"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_PIERCE_ARMOR")) ARMORED_ELEPHANT_PIERCE_ARMOR = Cast(std::get<0>(result["ARMORED_ELEPHANT_PIERCE_ARMOR"])).ToI32();
    if (result.count("ARMORED_ELEPHANT_ATTACK_INTERVAL")) ARMORED_ELEPHANT_ATTACK_INTERVAL = Double(std::get<0>(result["ARMORED_ELEPHANT_ATTACK_INTERVAL"]).c_str());
    if (result.count("ARMORED_ELEPHANT_ATTACK")) ARMORED_ELEPHANT_ATTACK = Cast(std::get<0>(result["ARMORED_ELEPHANT_ATTACK"])).ToI32();

    // 攻城武器
    if (result.count("STONE_THROWER_COST_FOOD")) STONE_THROWER_COST_FOOD = Cast(std::get<0>(result["STONE_THROWER_COST_FOOD"])).ToI32();
    if (result.count("STONE_THROWER_COST_WOOD")) STONE_THROWER_COST_WOOD = Cast(std::get<0>(result["STONE_THROWER_COST_WOOD"])).ToI32();
    if (result.count("STONE_THROWER_COST_GOLD")) STONE_THROWER_COST_GOLD = Cast(std::get<0>(result["STONE_THROWER_COST_GOLD"])).ToI32();
    if (result.count("STONE_THROWER_COST_STONE")) STONE_THROWER_COST_STONE = Cast(std::get<0>(result["STONE_THROWER_COST_STONE"])).ToI32();
    if (result.count("STONE_THROWER_TRAIN_TIME")) STONE_THROWER_TRAIN_TIME = Cast(std::get<0>(result["STONE_THROWER_TRAIN_TIME"])).ToI32();
    if (result.count("STONE_THROWER_HP")) STONE_THROWER_HP = Cast(std::get<0>(result["STONE_THROWER_HP"])).ToI32();
    if (result.count("STONE_THROWER_SPEED")) STONE_THROWER_SPEED = Double(std::get<0>(result["STONE_THROWER_SPEED"]).c_str());
    if (result.count("STONE_THROWER_LINE_OF_SIGHT")) STONE_THROWER_LINE_OF_SIGHT = Cast(std::get<0>(result["STONE_THROWER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("STONE_THROWER_HAND_ARMOR")) STONE_THROWER_HAND_ARMOR = Cast(std::get<0>(result["STONE_THROWER_HAND_ARMOR"])).ToI32();
    if (result.count("STONE_THROWER_PIERCE_ARMOR")) STONE_THROWER_PIERCE_ARMOR = Cast(std::get<0>(result["STONE_THROWER_PIERCE_ARMOR"])).ToI32();
    if (result.count("STONE_THROWER_RANGE")) STONE_THROWER_RANGE = Cast(std::get<0>(result["STONE_THROWER_RANGE"])).ToI32();
    if (result.count("STONE_THROWER_ATTACK_INTERVAL")) STONE_THROWER_ATTACK_INTERVAL = Double(std::get<0>(result["STONE_THROWER_ATTACK_INTERVAL"]).c_str());
    if (result.count("STONE_THROWER_ATTACK")) STONE_THROWER_ATTACK = Cast(std::get<0>(result["STONE_THROWER_ATTACK"])).ToI32();

    if (result.count("CATAPULT_COST_FOOD")) CATAPULT_COST_FOOD = Cast(std::get<0>(result["CATAPULT_COST_FOOD"])).ToI32();
    if (result.count("CATAPULT_COST_WOOD")) CATAPULT_COST_WOOD = Cast(std::get<0>(result["CATAPULT_COST_WOOD"])).ToI32();
    if (result.count("CATAPULT_COST_GOLD")) CATAPULT_COST_GOLD = Cast(std::get<0>(result["CATAPULT_COST_GOLD"])).ToI32();
    if (result.count("CATAPULT_COST_STONE")) CATAPULT_COST_STONE = Cast(std::get<0>(result["CATAPULT_COST_STONE"])).ToI32();
    if (result.count("CATAPULT_TRAIN_TIME")) CATAPULT_TRAIN_TIME = Cast(std::get<0>(result["CATAPULT_TRAIN_TIME"])).ToI32();
    if (result.count("CATAPULT_HP")) CATAPULT_HP = Cast(std::get<0>(result["CATAPULT_HP"])).ToI32();
    if (result.count("CATAPULT_SPEED")) CATAPULT_SPEED = Double(std::get<0>(result["CATAPULT_SPEED"]).c_str());
    if (result.count("CATAPULT_LINE_OF_SIGHT")) CATAPULT_LINE_OF_SIGHT = Cast(std::get<0>(result["CATAPULT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CATAPULT_HAND_ARMOR")) CATAPULT_HAND_ARMOR = Cast(std::get<0>(result["CATAPULT_HAND_ARMOR"])).ToI32();
    if (result.count("CATAPULT_PIERCE_ARMOR")) CATAPULT_PIERCE_ARMOR = Cast(std::get<0>(result["CATAPULT_PIERCE_ARMOR"])).ToI32();
    if (result.count("CATAPULT_RANGE")) CATAPULT_RANGE = Cast(std::get<0>(result["CATAPULT_RANGE"])).ToI32();
    if (result.count("CATAPULT_ATTACK_INTERVAL")) CATAPULT_ATTACK_INTERVAL = Double(std::get<0>(result["CATAPULT_ATTACK_INTERVAL"]).c_str());
    if (result.count("CATAPULT_ATTACK")) CATAPULT_ATTACK = Cast(std::get<0>(result["CATAPULT_ATTACK"])).ToI32();

    if (result.count("HEAVY_CATAPULT_COST_FOOD")) HEAVY_CATAPULT_COST_FOOD = Cast(std::get<0>(result["HEAVY_CATAPULT_COST_FOOD"])).ToI32();
    if (result.count("HEAVY_CATAPULT_COST_WOOD")) HEAVY_CATAPULT_COST_WOOD = Cast(std::get<0>(result["HEAVY_CATAPULT_COST_WOOD"])).ToI32();
    if (result.count("HEAVY_CATAPULT_COST_GOLD")) HEAVY_CATAPULT_COST_GOLD = Cast(std::get<0>(result["HEAVY_CATAPULT_COST_GOLD"])).ToI32();
    if (result.count("HEAVY_CATAPULT_COST_STONE")) HEAVY_CATAPULT_COST_STONE = Cast(std::get<0>(result["HEAVY_CATAPULT_COST_STONE"])).ToI32();
    if (result.count("HEAVY_CATAPULT_TRAIN_TIME")) HEAVY_CATAPULT_TRAIN_TIME = Cast(std::get<0>(result["HEAVY_CATAPULT_TRAIN_TIME"])).ToI32();
    if (result.count("HEAVY_CATAPULT_HP")) HEAVY_CATAPULT_HP = Cast(std::get<0>(result["HEAVY_CATAPULT_HP"])).ToI32();
    if (result.count("HEAVY_CATAPULT_SPEED")) HEAVY_CATAPULT_SPEED = Double(std::get<0>(result["HEAVY_CATAPULT_SPEED"]).c_str());
    if (result.count("HEAVY_CATAPULT_LINE_OF_SIGHT")) HEAVY_CATAPULT_LINE_OF_SIGHT = Cast(std::get<0>(result["HEAVY_CATAPULT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HEAVY_CATAPULT_HAND_ARMOR")) HEAVY_CATAPULT_HAND_ARMOR = Cast(std::get<0>(result["HEAVY_CATAPULT_HAND_ARMOR"])).ToI32();
    if (result.count("HEAVY_CATAPULT_PIERCE_ARMOR")) HEAVY_CATAPULT_PIERCE_ARMOR = Cast(std::get<0>(result["HEAVY_CATAPULT_PIERCE_ARMOR"])).ToI32();
    if (result.count("HEAVY_CATAPULT_RANGE")) HEAVY_CATAPULT_RANGE = Cast(std::get<0>(result["HEAVY_CATAPULT_RANGE"])).ToI32();
    if (result.count("HEAVY_CATAPULT_ATTACK_INTERVAL")) HEAVY_CATAPULT_ATTACK_INTERVAL = Double(std::get<0>(result["HEAVY_CATAPULT_ATTACK_INTERVAL"]).c_str());
    if (result.count("HEAVY_CATAPULT_ATTACK")) HEAVY_CATAPULT_ATTACK = Cast(std::get<0>(result["HEAVY_CATAPULT_ATTACK"])).ToI32();

    if (result.count("BALLISTA_COST_FOOD")) BALLISTA_COST_FOOD = Cast(std::get<0>(result["BALLISTA_COST_FOOD"])).ToI32();
    if (result.count("BALLISTA_COST_WOOD")) BALLISTA_COST_WOOD = Cast(std::get<0>(result["BALLISTA_COST_WOOD"])).ToI32();
    if (result.count("BALLISTA_COST_GOLD")) BALLISTA_COST_GOLD = Cast(std::get<0>(result["BALLISTA_COST_GOLD"])).ToI32();
    if (result.count("BALLISTA_COST_STONE")) BALLISTA_COST_STONE = Cast(std::get<0>(result["BALLISTA_COST_STONE"])).ToI32();
    if (result.count("BALLISTA_TRAIN_TIME")) BALLISTA_TRAIN_TIME = Cast(std::get<0>(result["BALLISTA_TRAIN_TIME"])).ToI32();
    if (result.count("BALLISTA_HP")) BALLISTA_HP = Cast(std::get<0>(result["BALLISTA_HP"])).ToI32();
    if (result.count("BALLISTA_SPEED")) BALLISTA_SPEED = Double(std::get<0>(result["BALLISTA_SPEED"]).c_str());
    if (result.count("BALLISTA_LINE_OF_SIGHT")) BALLISTA_LINE_OF_SIGHT = Cast(std::get<0>(result["BALLISTA_LINE_OF_SIGHT"])).ToI32();
    if (result.count("BALLISTA_HAND_ARMOR")) BALLISTA_HAND_ARMOR = Cast(std::get<0>(result["BALLISTA_HAND_ARMOR"])).ToI32();
    if (result.count("BALLISTA_PIERCE_ARMOR")) BALLISTA_PIERCE_ARMOR = Cast(std::get<0>(result["BALLISTA_PIERCE_ARMOR"])).ToI32();
    if (result.count("BALLISTA_RANGE")) BALLISTA_RANGE = Cast(std::get<0>(result["BALLISTA_RANGE"])).ToI32();
    if (result.count("BALLISTA_ATTACK_INTERVAL")) BALLISTA_ATTACK_INTERVAL = Double(std::get<0>(result["BALLISTA_ATTACK_INTERVAL"]).c_str());
    if (result.count("BALLISTA_ATTACK")) BALLISTA_ATTACK = Cast(std::get<0>(result["BALLISTA_ATTACK"])).ToI32();

    if (result.count("HELEPOLIS_COST_FOOD")) HELEPOLIS_COST_FOOD = Cast(std::get<0>(result["HELEPOLIS_COST_FOOD"])).ToI32();
    if (result.count("HELEPOLIS_COST_WOOD")) HELEPOLIS_COST_WOOD = Cast(std::get<0>(result["HELEPOLIS_COST_WOOD"])).ToI32();
    if (result.count("HELEPOLIS_COST_GOLD")) HELEPOLIS_COST_GOLD = Cast(std::get<0>(result["HELEPOLIS_COST_GOLD"])).ToI32();
    if (result.count("HELEPOLIS_COST_STONE")) HELEPOLIS_COST_STONE = Cast(std::get<0>(result["HELEPOLIS_COST_STONE"])).ToI32();
    if (result.count("HELEPOLIS_TRAIN_TIME")) HELEPOLIS_TRAIN_TIME = Cast(std::get<0>(result["HELEPOLIS_TRAIN_TIME"])).ToI32();
    if (result.count("HELEPOLIS_HP")) HELEPOLIS_HP = Cast(std::get<0>(result["HELEPOLIS_HP"])).ToI32();
    if (result.count("HELEPOLIS_SPEED")) HELEPOLIS_SPEED = Double(std::get<0>(result["HELEPOLIS_SPEED"]).c_str());
    if (result.count("HELEPOLIS_LINE_OF_SIGHT")) HELEPOLIS_LINE_OF_SIGHT = Cast(std::get<0>(result["HELEPOLIS_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HELEPOLIS_HAND_ARMOR")) HELEPOLIS_HAND_ARMOR = Cast(std::get<0>(result["HELEPOLIS_HAND_ARMOR"])).ToI32();
    if (result.count("HELEPOLIS_PIERCE_ARMOR")) HELEPOLIS_PIERCE_ARMOR = Cast(std::get<0>(result["HELEPOLIS_PIERCE_ARMOR"])).ToI32();
    if (result.count("HELEPOLIS_RANGE")) HELEPOLIS_RANGE = Cast(std::get<0>(result["HELEPOLIS_RANGE"])).ToI32();
    if (result.count("HELEPOLIS_ATTACK_INTERVAL")) HELEPOLIS_ATTACK_INTERVAL = Double(std::get<0>(result["HELEPOLIS_ATTACK_INTERVAL"]).c_str());
    if (result.count("HELEPOLIS_ATTACK")) HELEPOLIS_ATTACK = Cast(std::get<0>(result["HELEPOLIS_ATTACK"])).ToI32();

        // 船只
    if (result.count("FISHING_BOAT_COST_FOOD")) FISHING_BOAT_COST_FOOD = Cast(std::get<0>(result["FISHING_BOAT_COST_FOOD"])).ToI32();
    if (result.count("FISHING_BOAT_COST_WOOD")) FISHING_BOAT_COST_WOOD = Cast(std::get<0>(result["FISHING_BOAT_COST_WOOD"])).ToI32();
    if (result.count("FISHING_BOAT_COST_GOLD")) FISHING_BOAT_COST_GOLD = Cast(std::get<0>(result["FISHING_BOAT_COST_GOLD"])).ToI32();
    if (result.count("FISHING_BOAT_COST_STONE")) FISHING_BOAT_COST_STONE = Cast(std::get<0>(result["FISHING_BOAT_COST_STONE"])).ToI32();
    if (result.count("FISHING_BOAT_TRAIN_TIME")) FISHING_BOAT_TRAIN_TIME = Cast(std::get<0>(result["FISHING_BOAT_TRAIN_TIME"])).ToI32();
    if (result.count("FISHING_BOAT_HP")) FISHING_BOAT_HP = Cast(std::get<0>(result["FISHING_BOAT_HP"])).ToI32();
    if (result.count("FISHING_BOAT_SPEED")) FISHING_BOAT_SPEED = Double(std::get<0>(result["FISHING_BOAT_SPEED"]).c_str());
    if (result.count("FISHING_BOAT_LINE_OF_SIGHT")) FISHING_BOAT_LINE_OF_SIGHT = Cast(std::get<0>(result["FISHING_BOAT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("FISHING_BOAT_HAND_ARMOR")) FISHING_BOAT_HAND_ARMOR = Cast(std::get<0>(result["FISHING_BOAT_HAND_ARMOR"])).ToI32();
    if (result.count("FISHING_BOAT_PIERCE_ARMOR")) FISHING_BOAT_PIERCE_ARMOR = Cast(std::get<0>(result["FISHING_BOAT_PIERCE_ARMOR"])).ToI32();

    if (result.count("FISHING_SHIP_COST_FOOD")) FISHING_SHIP_COST_FOOD = Cast(std::get<0>(result["FISHING_SHIP_COST_FOOD"])).ToI32();
    if (result.count("FISHING_SHIP_COST_WOOD")) FISHING_SHIP_COST_WOOD = Cast(std::get<0>(result["FISHING_SHIP_COST_WOOD"])).ToI32();
    if (result.count("FISHING_SHIP_COST_GOLD")) FISHING_SHIP_COST_GOLD = Cast(std::get<0>(result["FISHING_SHIP_COST_GOLD"])).ToI32();
    if (result.count("FISHING_SHIP_COST_STONE")) FISHING_SHIP_COST_STONE = Cast(std::get<0>(result["FISHING_SHIP_COST_STONE"])).ToI32();
    if (result.count("FISHING_SHIP_TRAIN_TIME")) FISHING_SHIP_TRAIN_TIME = Cast(std::get<0>(result["FISHING_SHIP_TRAIN_TIME"])).ToI32();
    if (result.count("FISHING_SHIP_HP")) FISHING_SHIP_HP = Cast(std::get<0>(result["FISHING_SHIP_HP"])).ToI32();
    if (result.count("FISHING_SHIP_SPEED")) FISHING_SHIP_SPEED = Double(std::get<0>(result["FISHING_SHIP_SPEED"]).c_str());
    if (result.count("FISHING_SHIP_LINE_OF_SIGHT")) FISHING_SHIP_LINE_OF_SIGHT = Cast(std::get<0>(result["FISHING_SHIP_LINE_OF_SIGHT"])).ToI32();
    if (result.count("FISHING_SHIP_HAND_ARMOR")) FISHING_SHIP_HAND_ARMOR = Cast(std::get<0>(result["FISHING_SHIP_HAND_ARMOR"])).ToI32();
    if (result.count("FISHING_SHIP_PIERCE_ARMOR")) FISHING_SHIP_PIERCE_ARMOR = Cast(std::get<0>(result["FISHING_SHIP_PIERCE_ARMOR"])).ToI32();

    if (result.count("TRADE_BOAT_COST_FOOD")) TRADE_BOAT_COST_FOOD = Cast(std::get<0>(result["TRADE_BOAT_COST_FOOD"])).ToI32();
    if (result.count("TRADE_BOAT_COST_WOOD")) TRADE_BOAT_COST_WOOD = Cast(std::get<0>(result["TRADE_BOAT_COST_WOOD"])).ToI32();
    if (result.count("TRADE_BOAT_COST_GOLD")) TRADE_BOAT_COST_GOLD = Cast(std::get<0>(result["TRADE_BOAT_COST_GOLD"])).ToI32();
    if (result.count("TRADE_BOAT_COST_STONE")) TRADE_BOAT_COST_STONE = Cast(std::get<0>(result["TRADE_BOAT_COST_STONE"])).ToI32();
    if (result.count("TRADE_BOAT_TRAIN_TIME")) TRADE_BOAT_TRAIN_TIME = Cast(std::get<0>(result["TRADE_BOAT_TRAIN_TIME"])).ToI32();
    if (result.count("TRADE_BOAT_HP")) TRADE_BOAT_HP = Cast(std::get<0>(result["TRADE_BOAT_HP"])).ToI32();
    if (result.count("TRADE_BOAT_SPEED")) TRADE_BOAT_SPEED = Double(std::get<0>(result["TRADE_BOAT_SPEED"]).c_str());
    if (result.count("TRADE_BOAT_LINE_OF_SIGHT")) TRADE_BOAT_LINE_OF_SIGHT = Cast(std::get<0>(result["TRADE_BOAT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("TRADE_BOAT_HAND_ARMOR")) TRADE_BOAT_HAND_ARMOR = Cast(std::get<0>(result["TRADE_BOAT_HAND_ARMOR"])).ToI32();
    if (result.count("TRADE_BOAT_PIERCE_ARMOR")) TRADE_BOAT_PIERCE_ARMOR = Cast(std::get<0>(result["TRADE_BOAT_PIERCE_ARMOR"])).ToI32();

    if (result.count("MERCHANT_SHIP_COST_FOOD")) MERCHANT_SHIP_COST_FOOD = Cast(std::get<0>(result["MERCHANT_SHIP_COST_FOOD"])).ToI32();
    if (result.count("MERCHANT_SHIP_COST_WOOD")) MERCHANT_SHIP_COST_WOOD = Cast(std::get<0>(result["MERCHANT_SHIP_COST_WOOD"])).ToI32();
    if (result.count("MERCHANT_SHIP_COST_GOLD")) MERCHANT_SHIP_COST_GOLD = Cast(std::get<0>(result["MERCHANT_SHIP_COST_GOLD"])).ToI32();
    if (result.count("MERCHANT_SHIP_COST_STONE")) MERCHANT_SHIP_COST_STONE = Cast(std::get<0>(result["MERCHANT_SHIP_COST_STONE"])).ToI32();
    if (result.count("MERCHANT_SHIP_TRAIN_TIME")) MERCHANT_SHIP_TRAIN_TIME = Cast(std::get<0>(result["MERCHANT_SHIP_TRAIN_TIME"])).ToI32();
    if (result.count("MERCHANT_SHIP_HP")) MERCHANT_SHIP_HP = Cast(std::get<0>(result["MERCHANT_SHIP_HP"])).ToI32();
    if (result.count("MERCHANT_SHIP_SPEED")) MERCHANT_SHIP_SPEED = Double(std::get<0>(result["MERCHANT_SHIP_SPEED"]).c_str());
    if (result.count("MERCHANT_SHIP_LINE_OF_SIGHT")) MERCHANT_SHIP_LINE_OF_SIGHT = Cast(std::get<0>(result["MERCHANT_SHIP_LINE_OF_SIGHT"])).ToI32();
    if (result.count("MERCHANT_SHIP_HAND_ARMOR")) MERCHANT_SHIP_HAND_ARMOR = Cast(std::get<0>(result["MERCHANT_SHIP_HAND_ARMOR"])).ToI32();
    if (result.count("MERCHANT_SHIP_PIERCE_ARMOR")) MERCHANT_SHIP_PIERCE_ARMOR = Cast(std::get<0>(result["MERCHANT_SHIP_PIERCE_ARMOR"])).ToI32();

    if (result.count("LIGHT_TRANSPORT_COST_FOOD")) LIGHT_TRANSPORT_COST_FOOD = Cast(std::get<0>(result["LIGHT_TRANSPORT_COST_FOOD"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_COST_WOOD")) LIGHT_TRANSPORT_COST_WOOD = Cast(std::get<0>(result["LIGHT_TRANSPORT_COST_WOOD"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_COST_GOLD")) LIGHT_TRANSPORT_COST_GOLD = Cast(std::get<0>(result["LIGHT_TRANSPORT_COST_GOLD"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_COST_STONE")) LIGHT_TRANSPORT_COST_STONE = Cast(std::get<0>(result["LIGHT_TRANSPORT_COST_STONE"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_TRAIN_TIME")) LIGHT_TRANSPORT_TRAIN_TIME = Cast(std::get<0>(result["LIGHT_TRANSPORT_TRAIN_TIME"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_HP")) LIGHT_TRANSPORT_HP = Cast(std::get<0>(result["LIGHT_TRANSPORT_HP"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_SPEED")) LIGHT_TRANSPORT_SPEED = Double(std::get<0>(result["LIGHT_TRANSPORT_SPEED"]).c_str());
    if (result.count("LIGHT_TRANSPORT_LINE_OF_SIGHT")) LIGHT_TRANSPORT_LINE_OF_SIGHT = Cast(std::get<0>(result["LIGHT_TRANSPORT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_HAND_ARMOR")) LIGHT_TRANSPORT_HAND_ARMOR = Cast(std::get<0>(result["LIGHT_TRANSPORT_HAND_ARMOR"])).ToI32();
    if (result.count("LIGHT_TRANSPORT_PIERCE_ARMOR")) LIGHT_TRANSPORT_PIERCE_ARMOR = Cast(std::get<0>(result["LIGHT_TRANSPORT_PIERCE_ARMOR"])).ToI32();

    if (result.count("HEAVY_TRANSPORT_COST_FOOD")) HEAVY_TRANSPORT_COST_FOOD = Cast(std::get<0>(result["HEAVY_TRANSPORT_COST_FOOD"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_COST_WOOD")) HEAVY_TRANSPORT_COST_WOOD = Cast(std::get<0>(result["HEAVY_TRANSPORT_COST_WOOD"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_COST_GOLD")) HEAVY_TRANSPORT_COST_GOLD = Cast(std::get<0>(result["HEAVY_TRANSPORT_COST_GOLD"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_COST_STONE")) HEAVY_TRANSPORT_COST_STONE = Cast(std::get<0>(result["HEAVY_TRANSPORT_COST_STONE"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_TRAIN_TIME")) HEAVY_TRANSPORT_TRAIN_TIME = Cast(std::get<0>(result["HEAVY_TRANSPORT_TRAIN_TIME"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_HP")) HEAVY_TRANSPORT_HP = Cast(std::get<0>(result["HEAVY_TRANSPORT_HP"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_SPEED")) HEAVY_TRANSPORT_SPEED = Double(std::get<0>(result["HEAVY_TRANSPORT_SPEED"]).c_str());
    if (result.count("HEAVY_TRANSPORT_LINE_OF_SIGHT")) HEAVY_TRANSPORT_LINE_OF_SIGHT = Cast(std::get<0>(result["HEAVY_TRANSPORT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_HAND_ARMOR")) HEAVY_TRANSPORT_HAND_ARMOR = Cast(std::get<0>(result["HEAVY_TRANSPORT_HAND_ARMOR"])).ToI32();
    if (result.count("HEAVY_TRANSPORT_PIERCE_ARMOR")) HEAVY_TRANSPORT_PIERCE_ARMOR = Cast(std::get<0>(result["HEAVY_TRANSPORT_PIERCE_ARMOR"])).ToI32();

    if (result.count("SCOUT_SHIP_COST_FOOD")) SCOUT_SHIP_COST_FOOD = Cast(std::get<0>(result["SCOUT_SHIP_COST_FOOD"])).ToI32();
    if (result.count("SCOUT_SHIP_COST_WOOD")) SCOUT_SHIP_COST_WOOD = Cast(std::get<0>(result["SCOUT_SHIP_COST_WOOD"])).ToI32();
    if (result.count("SCOUT_SHIP_COST_GOLD")) SCOUT_SHIP_COST_GOLD = Cast(std::get<0>(result["SCOUT_SHIP_COST_GOLD"])).ToI32();
    if (result.count("SCOUT_SHIP_COST_STONE")) SCOUT_SHIP_COST_STONE = Cast(std::get<0>(result["SCOUT_SHIP_COST_STONE"])).ToI32();
    if (result.count("SCOUT_SHIP_TRAIN_TIME")) SCOUT_SHIP_TRAIN_TIME = Cast(std::get<0>(result["SCOUT_SHIP_TRAIN_TIME"])).ToI32();
    if (result.count("SCOUT_SHIP_HP")) SCOUT_SHIP_HP = Cast(std::get<0>(result["SCOUT_SHIP_HP"])).ToI32();
    if (result.count("SCOUT_SHIP_SPEED")) SCOUT_SHIP_SPEED = Double(std::get<0>(result["SCOUT_SHIP_SPEED"]).c_str());
    if (result.count("SCOUT_SHIP_LINE_OF_SIGHT")) SCOUT_SHIP_LINE_OF_SIGHT = Cast(std::get<0>(result["SCOUT_SHIP_LINE_OF_SIGHT"])).ToI32();
    if (result.count("SCOUT_SHIP_HAND_ARMOR")) SCOUT_SHIP_HAND_ARMOR = Cast(std::get<0>(result["SCOUT_SHIP_HAND_ARMOR"])).ToI32();
    if (result.count("SCOUT_SHIP_PIERCE_ARMOR")) SCOUT_SHIP_PIERCE_ARMOR = Cast(std::get<0>(result["SCOUT_SHIP_PIERCE_ARMOR"])).ToI32();
    if (result.count("SCOUT_SHIP_RANGE")) SCOUT_SHIP_RANGE = Cast(std::get<0>(result["SCOUT_SHIP_RANGE"])).ToI32();
    if (result.count("SCOUT_SHIP_ATTACK_INTERVAL")) SCOUT_SHIP_ATTACK_INTERVAL = Double(std::get<0>(result["SCOUT_SHIP_ATTACK_INTERVAL"]).c_str());
    if (result.count("SCOUT_SHIP_ATTACK")) SCOUT_SHIP_ATTACK = Cast(std::get<0>(result["SCOUT_SHIP_ATTACK"])).ToI32();

    if (result.count("WAR_GALLEY_COST_FOOD")) WAR_GALLEY_COST_FOOD = Cast(std::get<0>(result["WAR_GALLEY_COST_FOOD"])).ToI32();
    if (result.count("WAR_GALLEY_COST_WOOD")) WAR_GALLEY_COST_WOOD = Cast(std::get<0>(result["WAR_GALLEY_COST_WOOD"])).ToI32();
    if (result.count("WAR_GALLEY_COST_GOLD")) WAR_GALLEY_COST_GOLD = Cast(std::get<0>(result["WAR_GALLEY_COST_GOLD"])).ToI32();
    if (result.count("WAR_GALLEY_COST_STONE")) WAR_GALLEY_COST_STONE = Cast(std::get<0>(result["WAR_GALLEY_COST_STONE"])).ToI32();
    if (result.count("WAR_GALLEY_TRAIN_TIME")) WAR_GALLEY_TRAIN_TIME = Cast(std::get<0>(result["WAR_GALLEY_TRAIN_TIME"])).ToI32();
    if (result.count("WAR_GALLEY_HP")) WAR_GALLEY_HP = Cast(std::get<0>(result["WAR_GALLEY_HP"])).ToI32();
    if (result.count("WAR_GALLEY_SPEED")) WAR_GALLEY_SPEED = Double(std::get<0>(result["WAR_GALLEY_SPEED"]).c_str());
    if (result.count("WAR_GALLEY_LINE_OF_SIGHT")) WAR_GALLEY_LINE_OF_SIGHT = Cast(std::get<0>(result["WAR_GALLEY_LINE_OF_SIGHT"])).ToI32();
    if (result.count("WAR_GALLEY_HAND_ARMOR")) WAR_GALLEY_HAND_ARMOR = Cast(std::get<0>(result["WAR_GALLEY_HAND_ARMOR"])).ToI32();
    if (result.count("WAR_GALLEY_PIERCE_ARMOR")) WAR_GALLEY_PIERCE_ARMOR = Cast(std::get<0>(result["WAR_GALLEY_PIERCE_ARMOR"])).ToI32();
    if (result.count("WAR_GALLEY_RANGE")) WAR_GALLEY_RANGE = Cast(std::get<0>(result["WAR_GALLEY_RANGE"])).ToI32();
    if (result.count("WAR_GALLEY_ATTACK_INTERVAL")) WAR_GALLEY_ATTACK_INTERVAL = Double(std::get<0>(result["WAR_GALLEY_ATTACK_INTERVAL"]).c_str());
    if (result.count("WAR_GALLEY_ATTACK")) WAR_GALLEY_ATTACK = Cast(std::get<0>(result["WAR_GALLEY_ATTACK"])).ToI32();

    if (result.count("FIRE_GALLEY_COST_FOOD")) FIRE_GALLEY_COST_FOOD = Cast(std::get<0>(result["FIRE_GALLEY_COST_FOOD"])).ToI32();
    if (result.count("FIRE_GALLEY_COST_WOOD")) FIRE_GALLEY_COST_WOOD = Cast(std::get<0>(result["FIRE_GALLEY_COST_WOOD"])).ToI32();
    if (result.count("FIRE_GALLEY_COST_GOLD")) FIRE_GALLEY_COST_GOLD = Cast(std::get<0>(result["FIRE_GALLEY_COST_GOLD"])).ToI32();
    if (result.count("FIRE_GALLEY_COST_STONE")) FIRE_GALLEY_COST_STONE = Cast(std::get<0>(result["FIRE_GALLEY_COST_STONE"])).ToI32();
    if (result.count("FIRE_GALLEY_TRAIN_TIME")) FIRE_GALLEY_TRAIN_TIME = Cast(std::get<0>(result["FIRE_GALLEY_TRAIN_TIME"])).ToI32();
    if (result.count("FIRE_GALLEY_HP")) FIRE_GALLEY_HP = Cast(std::get<0>(result["FIRE_GALLEY_HP"])).ToI32();
    if (result.count("FIRE_GALLEY_SPEED")) FIRE_GALLEY_SPEED = Double(std::get<0>(result["FIRE_GALLEY_SPEED"]).c_str());
    if (result.count("FIRE_GALLEY_LINE_OF_SIGHT")) FIRE_GALLEY_LINE_OF_SIGHT = Cast(std::get<0>(result["FIRE_GALLEY_LINE_OF_SIGHT"])).ToI32();
    if (result.count("FIRE_GALLEY_HAND_ARMOR")) FIRE_GALLEY_HAND_ARMOR = Cast(std::get<0>(result["FIRE_GALLEY_HAND_ARMOR"])).ToI32();
    if (result.count("FIRE_GALLEY_PIERCE_ARMOR")) FIRE_GALLEY_PIERCE_ARMOR = Cast(std::get<0>(result["FIRE_GALLEY_PIERCE_ARMOR"])).ToI32();
    if (result.count("FIRE_GALLEY_RANGE")) FIRE_GALLEY_RANGE = Cast(std::get<0>(result["FIRE_GALLEY_RANGE"])).ToI32();
    if (result.count("FIRE_GALLEY_ATTACK_INTERVAL")) FIRE_GALLEY_ATTACK_INTERVAL = Double(std::get<0>(result["FIRE_GALLEY_ATTACK_INTERVAL"]).c_str());
    if (result.count("FIRE_GALLEY_ATTACK")) FIRE_GALLEY_ATTACK = Cast(std::get<0>(result["FIRE_GALLEY_ATTACK"])).ToI32();

    if (result.count("TRIREME_COST_FOOD")) TRIREME_COST_FOOD = Cast(std::get<0>(result["TRIREME_COST_FOOD"])).ToI32();
    if (result.count("TRIREME_COST_WOOD")) TRIREME_COST_WOOD = Cast(std::get<0>(result["TRIREME_COST_WOOD"])).ToI32();
    if (result.count("TRIREME_COST_GOLD")) TRIREME_COST_GOLD = Cast(std::get<0>(result["TRIREME_COST_GOLD"])).ToI32();
    if (result.count("TRIREME_COST_STONE")) TRIREME_COST_STONE = Cast(std::get<0>(result["TRIREME_COST_STONE"])).ToI32();
    if (result.count("TRIREME_TRAIN_TIME")) TRIREME_TRAIN_TIME = Cast(std::get<0>(result["TRIREME_TRAIN_TIME"])).ToI32();
    if (result.count("TRIREME_HP")) TRIREME_HP = Cast(std::get<0>(result["TRIREME_HP"])).ToI32();
    if (result.count("TRIREME_SPEED")) TRIREME_SPEED = Double(std::get<0>(result["TRIREME_SPEED"]).c_str());
    if (result.count("TRIREME_LINE_OF_SIGHT")) TRIREME_LINE_OF_SIGHT = Cast(std::get<0>(result["TRIREME_LINE_OF_SIGHT"])).ToI32();
    if (result.count("TRIREME_HAND_ARMOR")) TRIREME_HAND_ARMOR = Cast(std::get<0>(result["TRIREME_HAND_ARMOR"])).ToI32();
    if (result.count("TRIREME_PIERCE_ARMOR")) TRIREME_PIERCE_ARMOR = Cast(std::get<0>(result["TRIREME_PIERCE_ARMOR"])).ToI32();
    if (result.count("TRIREME_RANGE")) TRIREME_RANGE = Cast(std::get<0>(result["TRIREME_RANGE"])).ToI32();
    if (result.count("TRIREME_ATTACK_INTERVAL")) TRIREME_ATTACK_INTERVAL = Double(std::get<0>(result["TRIREME_ATTACK_INTERVAL"]).c_str());
    if (result.count("TRIREME_ATTACK")) TRIREME_ATTACK = Cast(std::get<0>(result["TRIREME_ATTACK"])).ToI32();

    if (result.count("CATAPULT_TRIREME_COST_FOOD")) CATAPULT_TRIREME_COST_FOOD = Cast(std::get<0>(result["CATAPULT_TRIREME_COST_FOOD"])).ToI32();
    if (result.count("CATAPULT_TRIREME_COST_WOOD")) CATAPULT_TRIREME_COST_WOOD = Cast(std::get<0>(result["CATAPULT_TRIREME_COST_WOOD"])).ToI32();
    if (result.count("CATAPULT_TRIREME_COST_GOLD")) CATAPULT_TRIREME_COST_GOLD = Cast(std::get<0>(result["CATAPULT_TRIREME_COST_GOLD"])).ToI32();
    if (result.count("CATAPULT_TRIREME_COST_STONE")) CATAPULT_TRIREME_COST_STONE = Cast(std::get<0>(result["CATAPULT_TRIREME_COST_STONE"])).ToI32();
    if (result.count("CATAPULT_TRIREME_TRAIN_TIME")) CATAPULT_TRIREME_TRAIN_TIME = Cast(std::get<0>(result["CATAPULT_TRIREME_TRAIN_TIME"])).ToI32();
    if (result.count("CATAPULT_TRIREME_HP")) CATAPULT_TRIREME_HP = Cast(std::get<0>(result["CATAPULT_TRIREME_HP"])).ToI32();
    if (result.count("CATAPULT_TRIREME_SPEED")) CATAPULT_TRIREME_SPEED = Double(std::get<0>(result["CATAPULT_TRIREME_SPEED"]).c_str());
    if (result.count("CATAPULT_TRIREME_LINE_OF_SIGHT")) CATAPULT_TRIREME_LINE_OF_SIGHT = Cast(std::get<0>(result["CATAPULT_TRIREME_LINE_OF_SIGHT"])).ToI32();
    if (result.count("CATAPULT_TRIREME_HAND_ARMOR")) CATAPULT_TRIREME_HAND_ARMOR = Cast(std::get<0>(result["CATAPULT_TRIREME_HAND_ARMOR"])).ToI32();
    if (result.count("CATAPULT_TRIREME_PIERCE_ARMOR")) CATAPULT_TRIREME_PIERCE_ARMOR = Cast(std::get<0>(result["CATAPULT_TRIREME_PIERCE_ARMOR"])).ToI32();
    if (result.count("CATAPULT_TRIREME_RANGE")) CATAPULT_TRIREME_RANGE = Cast(std::get<0>(result["CATAPULT_TRIREME_RANGE"])).ToI32();
    if (result.count("CATAPULT_TRIREME_ATTACK_INTERVAL")) CATAPULT_TRIREME_ATTACK_INTERVAL = Double(std::get<0>(result["CATAPULT_TRIREME_ATTACK_INTERVAL"]).c_str());
    if (result.count("CATAPULT_TRIREME_ATTACK")) CATAPULT_TRIREME_ATTACK = Cast(std::get<0>(result["CATAPULT_TRIREME_ATTACK"])).ToI32();

    if (result.count("JUGGERNAUT_COST_FOOD")) JUGGERNAUT_COST_FOOD = Cast(std::get<0>(result["JUGGERNAUT_COST_FOOD"])).ToI32();
    if (result.count("JUGGERNAUT_COST_WOOD")) JUGGERNAUT_COST_WOOD = Cast(std::get<0>(result["JUGGERNAUT_COST_WOOD"])).ToI32();
    if (result.count("JUGGERNAUT_COST_GOLD")) JUGGERNAUT_COST_GOLD = Cast(std::get<0>(result["JUGGERNAUT_COST_GOLD"])).ToI32();
    if (result.count("JUGGERNAUT_COST_STONE")) JUGGERNAUT_COST_STONE = Cast(std::get<0>(result["JUGGERNAUT_COST_STONE"])).ToI32();
    if (result.count("JUGGERNAUT_TRAIN_TIME")) JUGGERNAUT_TRAIN_TIME = Cast(std::get<0>(result["JUGGERNAUT_TRAIN_TIME"])).ToI32();
    if (result.count("JUGGERNAUT_HP")) JUGGERNAUT_HP = Cast(std::get<0>(result["JUGGERNAUT_HP"])).ToI32();
    if (result.count("JUGGERNAUT_SPEED")) JUGGERNAUT_SPEED = Double(std::get<0>(result["JUGGERNAUT_SPEED"]).c_str());
    if (result.count("JUGGERNAUT_LINE_OF_SIGHT")) JUGGERNAUT_LINE_OF_SIGHT = Cast(std::get<0>(result["JUGGERNAUT_LINE_OF_SIGHT"])).ToI32();
    if (result.count("JUGGERNAUT_HAND_ARMOR")) JUGGERNAUT_HAND_ARMOR = Cast(std::get<0>(result["JUGGERNAUT_HAND_ARMOR"])).ToI32();
    if (result.count("JUGGERNAUT_PIERCE_ARMOR")) JUGGERNAUT_PIERCE_ARMOR = Cast(std::get<0>(result["JUGGERNAUT_PIERCE_ARMOR"])).ToI32();
    if (result.count("JUGGERNAUT_RANGE")) JUGGERNAUT_RANGE = Cast(std::get<0>(result["JUGGERNAUT_RANGE"])).ToI32();
    if (result.count("JUGGERNAUT_ATTACK_INTERVAL")) JUGGERNAUT_ATTACK_INTERVAL = Double(std::get<0>(result["JUGGERNAUT_ATTACK_INTERVAL"]).c_str());
    if (result.count("JUGGERNAUT_ATTACK")) JUGGERNAUT_ATTACK = Cast(std::get<0>(result["JUGGERNAUT_ATTACK"])).ToI32();

    // 防御塔
    if (result.count("WATCH_TOWER_COST_FOOD")) WATCH_TOWER_COST_FOOD = Cast(std::get<0>(result["WATCH_TOWER_COST_FOOD"])).ToI32();
    if (result.count("WATCH_TOWER_COST_WOOD")) WATCH_TOWER_COST_WOOD = Cast(std::get<0>(result["WATCH_TOWER_COST_WOOD"])).ToI32();
    if (result.count("WATCH_TOWER_COST_GOLD")) WATCH_TOWER_COST_GOLD = Cast(std::get<0>(result["WATCH_TOWER_COST_GOLD"])).ToI32();
    if (result.count("WATCH_TOWER_COST_STONE")) WATCH_TOWER_COST_STONE = Cast(std::get<0>(result["WATCH_TOWER_COST_STONE"])).ToI32();
    if (result.count("WATCH_TOWER_TRAIN_TIME")) WATCH_TOWER_TRAIN_TIME = Cast(std::get<0>(result["WATCH_TOWER_TRAIN_TIME"])).ToI32();
    if (result.count("WATCH_TOWER_HP")) WATCH_TOWER_HP = Cast(std::get<0>(result["WATCH_TOWER_HP"])).ToI32();
    if (result.count("WATCH_TOWER_LINE_OF_SIGHT")) WATCH_TOWER_LINE_OF_SIGHT = Cast(std::get<0>(result["WATCH_TOWER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("WATCH_TOWER_PIERCE_ARMOR")) WATCH_TOWER_PIERCE_ARMOR = Cast(std::get<0>(result["WATCH_TOWER_PIERCE_ARMOR"])).ToI32();
    if (result.count("WATCH_TOWER_RANGE")) WATCH_TOWER_RANGE = Cast(std::get<0>(result["WATCH_TOWER_RANGE"])).ToI32();
    if (result.count("WATCH_TOWER_ATTACK_INTERVAL")) WATCH_TOWER_ATTACK_INTERVAL = Double(std::get<0>(result["WATCH_TOWER_ATTACK_INTERVAL"]).c_str());
    if (result.count("WATCH_TOWER_ATTACK")) WATCH_TOWER_ATTACK = Cast(std::get<0>(result["WATCH_TOWER_ATTACK"])).ToI32();

    if (result.count("SENTRY_TOWER_COST_FOOD")) SENTRY_TOWER_COST_FOOD = Cast(std::get<0>(result["SENTRY_TOWER_COST_FOOD"])).ToI32();
    if (result.count("SENTRY_TOWER_COST_WOOD")) SENTRY_TOWER_COST_WOOD = Cast(std::get<0>(result["SENTRY_TOWER_COST_WOOD"])).ToI32();
    if (result.count("SENTRY_TOWER_COST_GOLD")) SENTRY_TOWER_COST_GOLD = Cast(std::get<0>(result["SENTRY_TOWER_COST_GOLD"])).ToI32();
    if (result.count("SENTRY_TOWER_COST_STONE")) SENTRY_TOWER_COST_STONE = Cast(std::get<0>(result["SENTRY_TOWER_COST_STONE"])).ToI32();
    if (result.count("SENTRY_TOWER_TRAIN_TIME")) SENTRY_TOWER_TRAIN_TIME = Cast(std::get<0>(result["SENTRY_TOWER_TRAIN_TIME"])).ToI32();
    if (result.count("SENTRY_TOWER_HP")) SENTRY_TOWER_HP = Cast(std::get<0>(result["SENTRY_TOWER_HP"])).ToI32();
    if (result.count("SENTRY_TOWER_LINE_OF_SIGHT")) SENTRY_TOWER_LINE_OF_SIGHT = Cast(std::get<0>(result["SENTRY_TOWER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("SENTRY_TOWER_PIERCE_ARMOR")) SENTRY_TOWER_PIERCE_ARMOR = Cast(std::get<0>(result["SENTRY_TOWER_PIERCE_ARMOR"])).ToI32();
    if (result.count("SENTRY_TOWER_RANGE")) SENTRY_TOWER_RANGE = Cast(std::get<0>(result["SENTRY_TOWER_RANGE"])).ToI32();
    if (result.count("SENTRY_TOWER_ATTACK_INTERVAL")) SENTRY_TOWER_ATTACK_INTERVAL = Double(std::get<0>(result["SENTRY_TOWER_ATTACK_INTERVAL"]).c_str());
    if (result.count("SENTRY_TOWER_ATTACK")) SENTRY_TOWER_ATTACK = Cast(std::get<0>(result["SENTRY_TOWER_ATTACK"])).ToI32();

    if (result.count("GUARD_TOWER_COST_FOOD")) GUARD_TOWER_COST_FOOD = Cast(std::get<0>(result["GUARD_TOWER_COST_FOOD"])).ToI32();
    if (result.count("GUARD_TOWER_COST_WOOD")) GUARD_TOWER_COST_WOOD = Cast(std::get<0>(result["GUARD_TOWER_COST_WOOD"])).ToI32();
    if (result.count("GUARD_TOWER_COST_GOLD")) GUARD_TOWER_COST_GOLD = Cast(std::get<0>(result["GUARD_TOWER_COST_GOLD"])).ToI32();
    if (result.count("GUARD_TOWER_COST_STONE")) GUARD_TOWER_COST_STONE = Cast(std::get<0>(result["GUARD_TOWER_COST_STONE"])).ToI32();
    if (result.count("GUARD_TOWER_TRAIN_TIME")) GUARD_TOWER_TRAIN_TIME = Cast(std::get<0>(result["GUARD_TOWER_TRAIN_TIME"])).ToI32();
    if (result.count("GUARD_TOWER_HP")) GUARD_TOWER_HP = Cast(std::get<0>(result["GUARD_TOWER_HP"])).ToI32();
    if (result.count("GUARD_TOWER_LINE_OF_SIGHT")) GUARD_TOWER_LINE_OF_SIGHT = Cast(std::get<0>(result["GUARD_TOWER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("GUARD_TOWER_PIERCE_ARMOR")) GUARD_TOWER_PIERCE_ARMOR = Cast(std::get<0>(result["GUARD_TOWER_PIERCE_ARMOR"])).ToI32();
    if (result.count("GUARD_TOWER_RANGE")) GUARD_TOWER_RANGE = Cast(std::get<0>(result["GUARD_TOWER_RANGE"])).ToI32();
    if (result.count("GUARD_TOWER_ATTACK_INTERVAL")) GUARD_TOWER_ATTACK_INTERVAL = Double(std::get<0>(result["GUARD_TOWER_ATTACK_INTERVAL"]).c_str());
    if (result.count("GUARD_TOWER_ATTACK")) GUARD_TOWER_ATTACK = Cast(std::get<0>(result["GUARD_TOWER_ATTACK"])).ToI32();

    if (result.count("BALLISTA_TOWER_COST_FOOD")) BALLISTA_TOWER_COST_FOOD = Cast(std::get<0>(result["BALLISTA_TOWER_COST_FOOD"])).ToI32();
    if (result.count("BALLISTA_TOWER_COST_WOOD")) BALLISTA_TOWER_COST_WOOD = Cast(std::get<0>(result["BALLISTA_TOWER_COST_WOOD"])).ToI32();
    if (result.count("BALLISTA_TOWER_COST_GOLD")) BALLISTA_TOWER_COST_GOLD = Cast(std::get<0>(result["BALLISTA_TOWER_COST_GOLD"])).ToI32();
    if (result.count("BALLISTA_TOWER_COST_STONE")) BALLISTA_TOWER_COST_STONE = Cast(std::get<0>(result["BALLISTA_TOWER_COST_STONE"])).ToI32();
    if (result.count("BALLISTA_TOWER_TRAIN_TIME")) BALLISTA_TOWER_TRAIN_TIME = Cast(std::get<0>(result["BALLISTA_TOWER_TRAIN_TIME"])).ToI32();
    if (result.count("BALLISTA_TOWER_HP")) BALLISTA_TOWER_HP = Cast(std::get<0>(result["BALLISTA_TOWER_HP"])).ToI32();
    if (result.count("BALLISTA_TOWER_LINE_OF_SIGHT")) BALLISTA_TOWER_LINE_OF_SIGHT = Cast(std::get<0>(result["BALLISTA_TOWER_LINE_OF_SIGHT"])).ToI32();
    if (result.count("BALLISTA_TOWER_PIERCE_ARMOR")) BALLISTA_TOWER_PIERCE_ARMOR = Cast(std::get<0>(result["BALLISTA_TOWER_PIERCE_ARMOR"])).ToI32();
    if (result.count("BALLISTA_TOWER_RANGE")) BALLISTA_TOWER_RANGE = Cast(std::get<0>(result["BALLISTA_TOWER_RANGE"])).ToI32();
    if (result.count("BALLISTA_TOWER_ATTACK_INTERVAL")) BALLISTA_TOWER_ATTACK_INTERVAL = Double(std::get<0>(result["BALLISTA_TOWER_ATTACK_INTERVAL"]).c_str());
    if (result.count("BALLISTA_TOWER_ATTACK")) BALLISTA_TOWER_ATTACK = Cast(std::get<0>(result["BALLISTA_TOWER_ATTACK"])).ToI32();

    
}