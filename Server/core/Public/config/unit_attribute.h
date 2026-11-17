#pragma once
#include "Server/core/type.h"

// ==========================================
// 1. 科技属性 (Technology)
// ==========================================
extern I32 TOOLWORKING_COST_FOOD; // 工具制造术 食物成本
extern I32 TOOLWORKING_COST_WOOD; // 工具制造术 木材成本
extern I32 TOOLWORKING_COST_GOLD; // 工具制造术 黄金成本
extern I32 TOOLWORKING_COST_STONE; // 工具制造术 石材成本
extern I32 TOOLWORKING_RESEARCH_TIME; // 工具制造术 研发时间

extern I32 METALWORKING_COST_FOOD; // 金属加工术 食物成本
extern I32 METALWORKING_COST_WOOD; // 金属加工术 木材成本
extern I32 METALWORKING_COST_GOLD; // 金属加工术 黄金成本
extern I32 METALWORKING_COST_STONE; // 金属加工术 石材成本
extern I32 METALWORKING_RESEARCH_TIME; // 金属加工术 研发时间

extern I32 METALURGY_COST_FOOD; // 冶金术 食物成本
extern I32 METALURGY_COST_WOOD; // 冶金术 木材成本
extern I32 METALURGY_COST_GOLD; // 冶金术 黄金成本
extern I32 METALURGY_COST_STONE; // 冶金术 石材成本
extern I32 METALURGY_RESEARCH_TIME; // 冶金术 研发时间

extern I32 BRONZE_SHIELD_COST_FOOD; // 青铜盾 食物成本
extern I32 BRONZE_SHIELD_COST_WOOD; // 青铜盾 木材成本
extern I32 BRONZE_SHIELD_COST_GOLD; // 青铜盾 黄金成本
extern I32 BRONZE_SHIELD_COST_STONE; // 青铜盾 石材成本
extern I32 BRONZE_SHIELD_RESEARCH_TIME; // 青铜盾 研发时间

extern I32 IRON_SHIELD_COST_FOOD; // 铁盾 食物成本
extern I32 IRON_SHIELD_COST_WOOD; // 铁盾 木材成本
extern I32 IRON_SHIELD_COST_GOLD; // 铁盾 黄金成本
extern I32 IRON_SHIELD_COST_STONE; // 铁盾 石材成本
extern I32 IRON_SHIELD_RESEARCH_TIME; // 铁盾 研发时间

extern I32 TOWER_SHIELD_COST_FOOD; // 塔盾 食物成本
extern I32 TOWER_SHIELD_COST_WOOD; // 塔盾 木材成本
extern I32 TOWER_SHIELD_COST_GOLD; // 塔盾 黄金成本
extern I32 TOWER_SHIELD_COST_STONE; // 塔盾 石材成本
extern I32 TOWER_SHIELD_RESEARCH_TIME; // 塔盾 研发时间

// 其他护甲与科技省略部分，你可依样画葫芦或等完整版...

// ==========================================
// 2. 时代升级 (Age)
// ==========================================
extern I32 TOOL_AGE_COST_FOOD; // 石器时代 食物成本
extern I32 TOOL_AGE_COST_WOOD; // 石器时代 木材成本
extern I32 TOOL_AGE_COST_GOLD; // 石器时代 黄金成本
extern I32 TOOL_AGE_COST_STONE; // 石器时代 石材成本
extern I32 TOOL_AGE_RESEARCH_TIME; // 石器时代 研发时间

extern I32 BRONZE_AGE_COST_FOOD; // 青铜时代 食物成本
extern I32 BRONZE_AGE_COST_WOOD; // 青铜时代 木材成本
extern I32 BRONZE_AGE_COST_GOLD; // 青铜时代 黄金成本
extern I32 BRONZE_AGE_COST_STONE; // 青铜时代 石材成本
extern I32 BRONZE_AGE_RESEARCH_TIME; // 青铜时代 研发时间

extern I32 IRON_AGE_COST_FOOD; // 铁器时代 食物成本
extern I32 IRON_AGE_COST_WOOD; // 铁器时代 木材成本
extern I32 IRON_AGE_COST_GOLD; // 铁器时代 黄金成本
extern I32 IRON_AGE_COST_STONE; // 铁器时代 石材成本
extern I32 IRON_AGE_RESEARCH_TIME; // 铁器时代 研发时间

// ==========================================
// 3. 单位属性 (Units) - 完整兵种列表
// ==========================================

// --- 村民 (Villager) ---
extern I32 VILLAGER_COST_FOOD; // 村民 食物成本
extern I32 VILLAGER_COST_WOOD; // 村民 木材成本
extern I32 VILLAGER_COST_GOLD; // 村民 黄金成本
extern I32 VILLAGER_COST_STONE; // 村民 石材成本
extern I32 VILLAGER_TRAIN_TIME; // 村民 生产时间
extern I32 VILLAGER_HP; // 村民 生命值
extern Double VILLAGER_SPEED; // 村民 移动速度
extern I32 VILLAGER_LINE_OF_SIGHT; // 村民 视野
extern I32 VILLAGER_HAND_ARMOR; // 村民 肉搏护甲
extern I32 VILLAGER_PIERCE_ARMOR; // 村民 投射护甲
extern Double VILLAGER_ATTACK_INTERVAL; // 村民 攻击间距
extern I32 VILLAGER_ATTACK; // 村民 攻击力

// --- 牧师 (Priest) ---
extern I32 PRIEST_COST_FOOD; // 牧师 食物成本
extern I32 PRIEST_COST_WOOD; // 牧师 木材成本
extern I32 PRIEST_COST_GOLD; // 牧师 黄金成本
extern I32 PRIEST_COST_STONE; // 牧师 石材成本
extern I32 PRIEST_TRAIN_TIME; // 牧师 生产时间
extern I32 PRIEST_HP; // 牧师 生命值
extern Double PRIEST_SPEED; // 牧师 移动速度
extern I32 PRIEST_LINE_OF_SIGHT; // 牧师 视野
extern I32 PRIEST_HAND_ARMOR; // 牧师 肉搏护甲
extern I32 PRIEST_PIERCE_ARMOR; // 牧师 投射护甲
extern I32 PRIEST_RANGE; // 牧师 攻击距离
extern Double PRIEST_ATTACK_INTERVAL; // 牧师 攻击间距

// --- 步兵系列 ---
extern I32 CLUBMAN_COST_FOOD; // 石棍兵 食物成本
extern I32 CLUBMAN_COST_WOOD; // 石棍兵 木材成本
extern I32 CLUBMAN_COST_GOLD; // 石棍兵 黄金成本
extern I32 CLUBMAN_COST_STONE; // 石棍兵 石材成本
extern I32 CLUBMAN_TRAIN_TIME; // 石棍兵 生产时间
extern I32 CLUBMAN_HP; // 石棍兵 生命值
extern Double CLUBMAN_SPEED; // 石棍兵 移动速度
extern I32 CLUBMAN_LINE_OF_SIGHT; // 石棍兵 视野
extern I32 CLUBMAN_HAND_ARMOR; // 石棍兵 肉搏护甲
extern I32 CLUBMAN_PIERCE_ARMOR; // 石棍兵 投射护甲
extern Double CLUBMAN_ATTACK_INTERVAL; // 石棍兵 攻击间距
extern I32 CLUBMAN_ATTACK; // 石棍兵 攻击力

extern I32 AXEMAN_COST_FOOD; // 斧兵 食物成本
extern I32 AXEMAN_COST_WOOD; // 斧兵 木材成本
extern I32 AXEMAN_COST_GOLD; // 斧兵 黄金成本
extern I32 AXEMAN_COST_STONE; // 斧兵 石材成本
extern I32 AXEMAN_TRAIN_TIME; // 斧兵 生产时间
extern I32 AXEMAN_HP; // 斧兵 生命值
extern Double AXEMAN_SPEED; // 斧兵 移动速度
extern I32 AXEMAN_LINE_OF_SIGHT; // 斧兵 视野
extern I32 AXEMAN_HAND_ARMOR; // 斧兵 肉搏护甲
extern I32 AXEMAN_PIERCE_ARMOR; // 斧兵 投射护甲
extern Double AXEMAN_ATTACK_INTERVAL; // 斧兵 攻击间距
extern I32 AXEMAN_ATTACK; // 斧兵 攻击力

extern I32 SLINGER_COST_FOOD; // 投石兵 食物成本
extern I32 SLINGER_COST_WOOD; // 投石兵 木材成本
extern I32 SLINGER_COST_GOLD; // 投石兵 黄金成本
extern I32 SLINGER_COST_STONE; // 投石兵 石材成本
extern I32 SLINGER_TRAIN_TIME; // 投石兵 生产时间
extern I32 SLINGER_HP; // 投石兵 生命值
extern Double SLINGER_SPEED; // 投石兵 移动速度
extern I32 SLINGER_LINE_OF_SIGHT; // 投石兵 视野
extern I32 SLINGER_HAND_ARMOR; // 投石兵 肉搏护甲
extern I32 SLINGER_PIERCE_ARMOR; // 投石兵 投射护甲
extern I32 SLINGER_RANGE; // 投石兵 攻击距离
extern Double SLINGER_ATTACK_INTERVAL; // 投石兵 攻击间距
extern I32 SLINGER_ATTACK; // 投石兵 攻击力

extern I32 SHORT_SWORDSMAN_COST_FOOD; // 短剑兵 食物成本
extern I32 SHORT_SWORDSMAN_COST_WOOD; // 短剑兵 木材成本
extern I32 SHORT_SWORDSMAN_COST_GOLD; // 短剑兵 黄金成本
extern I32 SHORT_SWORDSMAN_COST_STONE; // 短剑兵 石材成本
extern I32 SHORT_SWORDSMAN_TRAIN_TIME; // 短剑兵 生产时间
extern I32 SHORT_SWORDSMAN_HP; // 短剑兵 生命值
extern Double SHORT_SWORDSMAN_SPEED; // 短剑兵 移动速度
extern I32 SHORT_SWORDSMAN_LINE_OF_SIGHT; // 短剑兵 视野
extern I32 SHORT_SWORDSMAN_HAND_ARMOR; // 短剑兵 肉搏护甲
extern I32 SHORT_SWORDSMAN_PIERCE_ARMOR; // 短剑兵 投射护甲
extern Double SHORT_SWORDSMAN_ATTACK_INTERVAL; // 短剑兵 攻击间距
extern I32 SHORT_SWORDSMAN_ATTACK; // 短剑兵 攻击力

extern I32 BROAD_SWORDSMAN_COST_FOOD; // 阔剑兵 食物成本
extern I32 BROAD_SWORDSMAN_COST_WOOD; // 阔剑兵 木材成本
extern I32 BROAD_SWORDSMAN_COST_GOLD; // 阔剑兵 黄金成本
extern I32 BROAD_SWORDSMAN_COST_STONE; // 阔剑兵 石材成本
extern I32 BROAD_SWORDSMAN_TRAIN_TIME; // 阔剑兵 生产时间
extern I32 BROAD_SWORDSMAN_HP; // 阔剑兵 生命值
extern Double BROAD_SWORDSMAN_SPEED; // 阔剑兵 移动速度
extern I32 BROAD_SWORDSMAN_LINE_OF_SIGHT; // 阔剑兵 视野
extern I32 BROAD_SWORDSMAN_HAND_ARMOR; // 阔剑兵 肉搏护甲
extern I32 BROAD_SWORDSMAN_PIERCE_ARMOR; // 阔剑兵 投射护甲
extern Double BROAD_SWORDSMAN_ATTACK_INTERVAL; // 阔剑兵 攻击间距
extern I32 BROAD_SWORDSMAN_ATTACK; // 阔剑兵 攻击力

extern I32 LONG_SWORDSMAN_COST_FOOD; // 长剑兵 食物成本
extern I32 LONG_SWORDSMAN_COST_WOOD; // 长剑兵 木材成本
extern I32 LONG_SWORDSMAN_COST_GOLD; // 长剑兵 黄金成本
extern I32 LONG_SWORDSMAN_COST_STONE; // 长剑兵 石材成本
extern I32 LONG_SWORDSMAN_TRAIN_TIME; // 长剑兵 生产时间
extern I32 LONG_SWORDSMAN_HP; // 长剑兵 生命值
extern Double LONG_SWORDSMAN_SPEED; // 长剑兵 移动速度
extern I32 LONG_SWORDSMAN_LINE_OF_SIGHT; // 长剑兵 视野
extern I32 LONG_SWORDSMAN_HAND_ARMOR; // 长剑兵 肉搏护甲
extern I32 LONG_SWORDSMAN_PIERCE_ARMOR; // 长剑兵 投射护甲
extern Double LONG_SWORDSMAN_ATTACK_INTERVAL; // 长剑兵 攻击间距
extern I32 LONG_SWORDSMAN_ATTACK; // 长剑兵 攻击力

extern I32 LEGION_COST_FOOD; // 军团兵 食物成本
extern I32 LEGION_COST_WOOD; // 军团兵 木材成本
extern I32 LEGION_COST_GOLD; // 军团兵 黄金成本
extern I32 LEGION_COST_STONE; // 军团兵 石材成本
extern I32 LEGION_TRAIN_TIME; // 军团兵 生产时间
extern I32 LEGION_HP; // 军团兵 生命值
extern Double LEGION_SPEED; // 军团兵 移动速度
extern I32 LEGION_LINE_OF_SIGHT; // 军团兵 视野
extern I32 LEGION_HAND_ARMOR; // 军团兵 肉搏护甲
extern I32 LEGION_PIERCE_ARMOR; // 军团兵 投射护甲
extern Double LEGION_ATTACK_INTERVAL; // 军团兵 攻击间距
extern I32 LEGION_ATTACK; // 军团兵 攻击力

extern I32 HOPLITE_COST_FOOD; // 重装步兵 食物成本
extern I32 HOPLITE_COST_WOOD; // 重装步兵 木材成本
extern I32 HOPLITE_COST_GOLD; // 重装步兵 黄金成本
extern I32 HOPLITE_COST_STONE; // 重装步兵 石材成本
extern I32 HOPLITE_TRAIN_TIME; // 重装步兵 生产时间
extern I32 HOPLITE_HP; // 重装步兵 生命值
extern Double HOPLITE_SPEED; // 重装步兵 移动速度
extern I32 HOPLITE_LINE_OF_SIGHT; // 重装步兵 视野
extern I32 HOPLITE_HAND_ARMOR; // 重装步兵 肉搏护甲
extern I32 HOPLITE_PIERCE_ARMOR; // 重装步兵 投射护甲
extern Double HOPLITE_ATTACK_INTERVAL; // 重装步兵 攻击间距
extern I32 HOPLITE_ATTACK; // 重装步兵 攻击力

extern I32 PHALANX_COST_FOOD; // 方阵部队 食物成本
extern I32 PHALANX_COST_WOOD; // 方阵部队 木材成本
extern I32 PHALANX_COST_GOLD; // 方阵部队 黄金成本
extern I32 PHALANX_COST_STONE; // 方阵部队 石材成本
extern I32 PHALANX_TRAIN_TIME; // 方阵部队 生产时间
extern I32 PHALANX_HP; // 方阵部队 生命值
extern Double PHALANX_SPEED; // 方阵部队 移动速度
extern I32 PHALANX_LINE_OF_SIGHT; // 方阵部队 视野
extern I32 PHALANX_HAND_ARMOR; // 方阵部队 肉搏护甲
extern I32 PHALANX_PIERCE_ARMOR; // 方阵部队 投射护甲
extern Double PHALANX_ATTACK_INTERVAL; // 方阵部队 攻击间距
extern I32 PHALANX_ATTACK; // 方阵部队 攻击力

extern I32 CENTURION_COST_FOOD; // 百夫长 食物成本
extern I32 CENTURION_COST_WOOD; // 百夫长 木材成本
extern I32 CENTURION_COST_GOLD; // 百夫长 黄金成本
extern I32 CENTURION_COST_STONE; // 百夫长 石材成本
extern I32 CENTURION_TRAIN_TIME; // 百夫长 生产时间
extern I32 CENTURION_HP; // 百夫长 生命值
extern Double CENTURION_SPEED; // 百夫长 移动速度
extern I32 CENTURION_LINE_OF_SIGHT; // 百夫长 视野
extern I32 CENTURION_HAND_ARMOR; // 百夫长 肉搏护甲
extern I32 CENTURION_PIERCE_ARMOR; // 百夫长 投射护甲
extern Double CENTURION_ATTACK_INTERVAL; // 百夫长 攻击间距
extern I32 CENTURION_ATTACK; // 百夫长 攻击力

// --- 弓兵系列 ---
extern I32 BOWMAN_COST_FOOD; // 弓箭兵 食物成本
extern I32 BOWMAN_COST_WOOD; // 弓箭兵 木材成本
extern I32 BOWMAN_COST_GOLD; // 弓箭兵 黄金成本
extern I32 BOWMAN_COST_STONE; // 弓箭兵 石材成本
extern I32 BOWMAN_TRAIN_TIME; // 弓箭兵 生产时间
extern I32 BOWMAN_HP; // 弓箭兵 生命值
extern Double BOWMAN_SPEED; // 弓箭兵 移动速度
extern I32 BOWMAN_LINE_OF_SIGHT; // 弓箭兵 视野
extern I32 BOWMAN_HAND_ARMOR; // 弓箭兵 肉搏护甲
extern I32 BOWMAN_PIERCE_ARMOR; // 弓箭兵 投射护甲
extern I32 BOWMAN_RANGE; // 弓箭兵 攻击距离
extern Double BOWMAN_ATTACK_INTERVAL; // 弓箭兵 攻击间距
extern I32 BOWMAN_ATTACK; // 弓箭兵 攻击力

extern I32 IMPROVED_BOWMAN_COST_FOOD; // 强弓兵 食物成本
extern I32 IMPROVED_BOWMAN_COST_WOOD; // 强弓兵 木材成本
extern I32 IMPROVED_BOWMAN_COST_GOLD; // 强弓兵 黄金成本
extern I32 IMPROVED_BOWMAN_COST_STONE; // 强弓兵 石材成本
extern I32 IMPROVED_BOWMAN_TRAIN_TIME; // 强弓兵 生产时间
extern I32 IMPROVED_BOWMAN_HP; // 强弓兵 生命值
extern Double IMPROVED_BOWMAN_SPEED; // 强弓兵 移动速度
extern I32 IMPROVED_BOWMAN_LINE_OF_SIGHT; // 强弓兵 视野
extern I32 IMPROVED_BOWMAN_HAND_ARMOR; // 强弓兵 肉搏护甲
extern I32 IMPROVED_BOWMAN_PIERCE_ARMOR; // 强弓兵 投射护甲
extern I32 IMPROVED_BOWMAN_RANGE; // 强弓兵 攻击距离
extern Double IMPROVED_BOWMAN_ATTACK_INTERVAL; // 强弓兵 攻击间距
extern I32 IMPROVED_BOWMAN_ATTACK; // 强弓兵 攻击力

extern I32 COMPOSITE_BOWMAN_COST_FOOD; // 混合弓兵 食物成本
extern I32 COMPOSITE_BOWMAN_COST_WOOD; // 混合弓兵 木材成本
extern I32 COMPOSITE_BOWMAN_COST_GOLD; // 混合弓兵 黄金成本
extern I32 COMPOSITE_BOWMAN_COST_STONE; // 混合弓兵 石材成本
extern I32 COMPOSITE_BOWMAN_TRAIN_TIME; // 混合弓兵 生产时间
extern I32 COMPOSITE_BOWMAN_HP; // 混合弓兵 生命值
extern Double COMPOSITE_BOWMAN_SPEED; // 混合弓兵 移动速度
extern I32 COMPOSITE_BOWMAN_LINE_OF_SIGHT; // 混合弓兵 视野
extern I32 COMPOSITE_BOWMAN_HAND_ARMOR; // 混合弓兵 肉搏护甲
extern I32 COMPOSITE_BOWMAN_PIERCE_ARMOR; // 混合弓兵 投射护甲
extern I32 COMPOSITE_BOWMAN_RANGE; // 混合弓兵 攻击距离
extern Double COMPOSITE_BOWMAN_ATTACK_INTERVAL; // 混合弓兵 攻击间距
extern I32 COMPOSITE_BOWMAN_ATTACK; // 混合弓兵 攻击力

extern I32 CHARIOT_ARCHER_COST_FOOD; // 战车弓兵 食物成本
extern I32 CHARIOT_ARCHER_COST_WOOD; // 战车弓兵 木材成本
extern I32 CHARIOT_ARCHER_COST_GOLD; // 战车弓兵 黄金成本
extern I32 CHARIOT_ARCHER_COST_STONE; // 战车弓兵 石材成本
extern I32 CHARIOT_ARCHER_TRAIN_TIME; // 战车弓兵 生产时间
extern I32 CHARIOT_ARCHER_HP; // 战车弓兵 生命值
extern Double CHARIOT_ARCHER_SPEED; // 战车弓兵 移动速度
extern I32 CHARIOT_ARCHER_LINE_OF_SIGHT; // 战车弓兵 视野
extern I32 CHARIOT_ARCHER_HAND_ARMOR; // 战车弓兵 肉搏护甲
extern I32 CHARIOT_ARCHER_PIERCE_ARMOR; // 战车弓兵 投射护甲
extern I32 CHARIOT_ARCHER_RANGE; // 战车弓兵 攻击距离
extern Double CHARIOT_ARCHER_ATTACK_INTERVAL; // 战车弓兵 攻击间距
extern I32 CHARIOT_ARCHER_ATTACK; // 战车弓兵 攻击力

extern I32 ELEPHANT_ARCHER_COST_FOOD; // 战象弓兵 食物成本
extern I32 ELEPHANT_ARCHER_COST_WOOD; // 战象弓兵 木材成本
extern I32 ELEPHANT_ARCHER_COST_GOLD; // 战象弓兵 黄金成本
extern I32 ELEPHANT_ARCHER_COST_STONE; // 战象弓兵 石材成本
extern I32 ELEPHANT_ARCHER_TRAIN_TIME; // 战象弓兵 生产时间
extern I32 ELEPHANT_ARCHER_HP; // 战象弓兵 生命值
extern Double ELEPHANT_ARCHER_SPEED; // 战象弓兵 移动速度
extern I32 ELEPHANT_ARCHER_LINE_OF_SIGHT; // 战象弓兵 视野
extern I32 ELEPHANT_ARCHER_HAND_ARMOR; // 战象弓兵 肉搏护甲
extern I32 ELEPHANT_ARCHER_PIERCE_ARMOR; // 战象弓兵 投射护甲
extern I32 ELEPHANT_ARCHER_RANGE; // 战象弓兵 攻击距离
extern Double ELEPHANT_ARCHER_ATTACK_INTERVAL; // 战象弓兵 攻击间距
extern I32 ELEPHANT_ARCHER_ATTACK; // 战象弓兵 攻击力

// --- 骑兵系列 ---
extern I32 HORSE_ARCHER_COST_FOOD; // 骑弓兵 食物成本
extern I32 HORSE_ARCHER_COST_WOOD; // 骑弓兵 木材成本
extern I32 HORSE_ARCHER_COST_GOLD; // 骑弓兵 黄金成本
extern I32 HORSE_ARCHER_COST_STONE; // 骑弓兵 石材成本
extern I32 HORSE_ARCHER_TRAIN_TIME; // 骑弓兵 生产时间
extern I32 HORSE_ARCHER_HP; // 骑弓兵 生命值
extern Double HORSE_ARCHER_SPEED; // 骑弓兵 移动速度
extern I32 HORSE_ARCHER_LINE_OF_SIGHT; // 骑弓兵 视野
extern I32 HORSE_ARCHER_HAND_ARMOR; // 骑弓兵 肉搏护甲
extern I32 HORSE_ARCHER_PIERCE_ARMOR; // 骑弓兵 投射护甲
extern I32 HORSE_ARCHER_RANGE; // 骑弓兵 攻击距离
extern Double HORSE_ARCHER_ATTACK_INTERVAL; // 骑弓兵 攻击间距
extern I32 HORSE_ARCHER_ATTACK; // 骑弓兵 攻击力

extern I32 HEAVY_HORSE_ARCHER_COST_FOOD; // 重骑弓兵 食物成本
extern I32 HEAVY_HORSE_ARCHER_COST_WOOD; // 重骑弓兵 木材成本
extern I32 HEAVY_HORSE_ARCHER_COST_GOLD; // 重骑弓兵 黄金成本
extern I32 HEAVY_HORSE_ARCHER_COST_STONE; // 重骑弓兵 石材成本
extern I32 HEAVY_HORSE_ARCHER_TRAIN_TIME; // 重骑弓兵 生产时间
extern I32 HEAVY_HORSE_ARCHER_HP; // 重骑弓兵 生命值
extern Double HEAVY_HORSE_ARCHER_SPEED; // 重骑弓兵 移动速度
extern I32 HEAVY_HORSE_ARCHER_LINE_OF_SIGHT; // 重骑弓兵 视野
extern I32 HEAVY_HORSE_ARCHER_HAND_ARMOR; // 重骑弓兵 肉搏护甲
extern I32 HEAVY_HORSE_ARCHER_PIERCE_ARMOR; // 重骑弓兵 投射护甲
extern I32 HEAVY_HORSE_ARCHER_RANGE; // 重骑弓兵 攻击距离
extern Double HEAVY_HORSE_ARCHER_ATTACK_INTERVAL; // 重骑弓兵 攻击间距
extern I32 HEAVY_HORSE_ARCHER_ATTACK; // 重骑弓兵 攻击力

extern I32 SCOUT_COST_FOOD; // 侦察骑兵 食物成本
extern I32 SCOUT_COST_WOOD; // 侦察骑兵 木材成本
extern I32 SCOUT_COST_GOLD; // 侦察骑兵 黄金成本
extern I32 SCOUT_COST_STONE; // 侦察骑兵 石材成本
extern I32 SCOUT_TRAIN_TIME; // 侦察骑兵 生产时间
extern I32 SCOUT_HP; // 侦察骑兵 生命值
extern Double SCOUT_SPEED; // 侦察骑兵 移动速度
extern I32 SCOUT_LINE_OF_SIGHT; // 侦察骑兵 视野
extern I32 SCOUT_HAND_ARMOR; // 侦察骑兵 肉搏护甲
extern I32 SCOUT_PIERCE_ARMOR; // 侦察骑兵 投射护甲
extern Double SCOUT_ATTACK_INTERVAL; // 侦察骑兵 攻击间距
extern I32 SCOUT_ATTACK; // 侦察骑兵 攻击力

extern I32 CAMEL_RIDER_COST_FOOD; // 骆驼兵 食物成本
extern I32 CAMEL_RIDER_COST_WOOD; // 骆驼兵 木材成本
extern I32 CAMEL_RIDER_COST_GOLD; // 骆驼兵 黄金成本
extern I32 CAMEL_RIDER_COST_STONE; // 骆驼兵 石材成本
extern I32 CAMEL_RIDER_TRAIN_TIME; // 骆驼兵 生产时间
extern I32 CAMEL_RIDER_HP; // 骆驼兵 生命值
extern Double CAMEL_RIDER_SPEED; // 骆驼兵 移动速度
extern I32 CAMEL_RIDER_LINE_OF_SIGHT; // 骆驼兵 视野
extern I32 CAMEL_RIDER_HAND_ARMOR; // 骆驼兵 肉搏护甲
extern I32 CAMEL_RIDER_PIERCE_ARMOR; // 骆驼兵 投射护甲
extern Double CAMEL_RIDER_ATTACK_INTERVAL; // 骆驼兵 攻击间距
extern I32 CAMEL_RIDER_ATTACK; // 骆驼兵 攻击力

extern I32 CHARIOT_COST_FOOD; // 两轮战车 食物成本
extern I32 CHARIOT_COST_WOOD; // 两轮战车 木材成本
extern I32 CHARIOT_COST_GOLD; // 两轮战车 黄金成本
extern I32 CHARIOT_COST_STONE; // 两轮战车 石材成本
extern I32 CHARIOT_TRAIN_TIME; // 两轮战车 生产时间
extern I32 CHARIOT_HP; // 两轮战车 生命值
extern Double CHARIOT_SPEED; // 两轮战车 移动速度
extern I32 CHARIOT_LINE_OF_SIGHT; // 两轮战车 视野
extern I32 CHARIOT_HAND_ARMOR; // 两轮战车 肉搏护甲
extern I32 CHARIOT_PIERCE_ARMOR; // 两轮战车 投射护甲
extern Double CHARIOT_ATTACK_INTERVAL; // 两轮战车 攻击间距
extern I32 CHARIOT_ATTACK; // 两轮战车 攻击力

extern I32 SCYTHE_CHARIOT_COST_FOOD; // 轮式镰刀战车 食物成本
extern I32 SCYTHE_CHARIOT_COST_WOOD; // 轮式镰刀战车 木材成本
extern I32 SCYTHE_CHARIOT_COST_GOLD; // 轮式镰刀战车 黄金成本
extern I32 SCYTHE_CHARIOT_COST_STONE; // 轮式镰刀战车 石材成本
extern I32 SCYTHE_CHARIOT_TRAIN_TIME; // 轮式镰刀战车 生产时间
extern I32 SCYTHE_CHARIOT_HP; // 轮式镰刀战车 生命值
extern Double SCYTHE_CHARIOT_SPEED; // 轮式镰刀战车 移动速度
extern I32 SCYTHE_CHARIOT_LINE_OF_SIGHT; // 轮式镰刀战车 视野
extern I32 SCYTHE_CHARIOT_HAND_ARMOR; // 轮式镰刀战车 肉搏护甲
extern I32 SCYTHE_CHARIOT_PIERCE_ARMOR; // 轮式镰刀战车 投射护甲
extern Double SCYTHE_CHARIOT_ATTACK_INTERVAL; // 轮式镰刀战车 攻击间距
extern I32 SCYTHE_CHARIOT_ATTACK; // 轮式镰刀战车 攻击力

extern I32 CAVALRY_COST_FOOD; // 骑兵 食物成本
extern I32 CAVALRY_COST_WOOD; // 骑兵 木材成本
extern I32 CAVALRY_COST_GOLD; // 骑兵 黄金成本
extern I32 CAVALRY_COST_STONE; // 骑兵 石材成本
extern I32 CAVALRY_TRAIN_TIME; // 骑兵 生产时间
extern I32 CAVALRY_HP; // 骑兵 生命值
extern Double CAVALRY_SPEED; // 骑兵 移动速度
extern I32 CAVALRY_LINE_OF_SIGHT; // 骑兵 视野
extern I32 CAVALRY_HAND_ARMOR; // 骑兵 肉搏护甲
extern I32 CAVALRY_PIERCE_ARMOR; // 骑兵 投射护甲
extern Double CAVALRY_ATTACK_INTERVAL; // 骑兵 攻击间距
extern I32 CAVALRY_ATTACK; // 骑兵 攻击力

extern I32 HEAVY_CAVALRY_COST_FOOD; // 重骑兵 食物成本
extern I32 HEAVY_CAVALRY_COST_WOOD; // 重骑兵 木材成本
extern I32 HEAVY_CAVALRY_COST_GOLD; // 重骑兵 黄金成本
extern I32 HEAVY_CAVALRY_COST_STONE; // 重骑兵 石材成本
extern I32 HEAVY_CAVALRY_TRAIN_TIME; // 重骑兵 生产时间
extern I32 HEAVY_CAVALRY_HP; // 重骑兵 生命值
extern Double HEAVY_CAVALRY_SPEED; // 重骑兵 移动速度
extern I32 HEAVY_CAVALRY_LINE_OF_SIGHT; // 重骑兵 视野
extern I32 HEAVY_CAVALRY_HAND_ARMOR; // 重骑兵 肉搏护甲
extern I32 HEAVY_CAVALRY_PIERCE_ARMOR; // 重骑兵 投射护甲
extern Double HEAVY_CAVALRY_ATTACK_INTERVAL; // 重骑兵 攻击间距
extern I32 HEAVY_CAVALRY_ATTACK; // 重骑兵 攻击力

extern I32 CATAPHRACT_COST_FOOD; // 圣骑兵 食物成本
extern I32 CATAPHRACT_COST_WOOD; // 圣骑兵 木材成本
extern I32 CATAPHRACT_COST_GOLD; // 圣骑兵 黄金成本
extern I32 CATAPHRACT_COST_STONE; // 圣骑兵 石材成本
extern I32 CATAPHRACT_TRAIN_TIME; // 圣骑兵 生产时间
extern I32 CATAPHRACT_HP; // 圣骑兵 生命值
extern Double CATAPHRACT_SPEED; // 圣骑兵 移动速度
extern I32 CATAPHRACT_LINE_OF_SIGHT; // 圣骑兵 视野
extern I32 CATAPHRACT_HAND_ARMOR; // 圣骑兵 肉搏护甲
extern I32 CATAPHRACT_PIERCE_ARMOR; // 圣骑兵 投射护甲
extern Double CATAPHRACT_ATTACK_INTERVAL; // 圣骑兵 攻击间距
extern I32 CATAPHRACT_ATTACK; // 圣骑兵 攻击力

// --- 战象系列 ---
extern I32 WAR_ELEPHANT_COST_FOOD; // 战象 食物成本
extern I32 WAR_ELEPHANT_COST_WOOD; // 战象 木材成本
extern I32 WAR_ELEPHANT_COST_GOLD; // 战象 黄金成本
extern I32 WAR_ELEPHANT_COST_STONE; // 战象 石材成本
extern I32 WAR_ELEPHANT_TRAIN_TIME; // 战象 生产时间
extern I32 WAR_ELEPHANT_HP; // 战象 生命值
extern Double WAR_ELEPHANT_SPEED; // 战象 移动速度
extern I32 WAR_ELEPHANT_LINE_OF_SIGHT; // 战象 视野
extern I32 WAR_ELEPHANT_HAND_ARMOR; // 战象 肉搏护甲
extern I32 WAR_ELEPHANT_PIERCE_ARMOR; // 战象 投射护甲
extern Double WAR_ELEPHANT_ATTACK_INTERVAL; // 战象 攻击间距
extern I32 WAR_ELEPHANT_ATTACK; // 战象 攻击力

extern I32 ARMORED_ELEPHANT_COST_FOOD; // 披甲战象 食物成本
extern I32 ARMORED_ELEPHANT_COST_WOOD; // 披甲战象 木材成本
extern I32 ARMORED_ELEPHANT_COST_GOLD; // 披甲战象 黄金成本
extern I32 ARMORED_ELEPHANT_COST_STONE; // 披甲战象 石材成本
extern I32 ARMORED_ELEPHANT_TRAIN_TIME; // 披甲战象 生产时间
extern I32 ARMORED_ELEPHANT_HP; // 披甲战象 生命值
extern Double ARMORED_ELEPHANT_SPEED; // 披甲战象 移动速度
extern I32 ARMORED_ELEPHANT_LINE_OF_SIGHT; // 披甲战象 视野
extern I32 ARMORED_ELEPHANT_HAND_ARMOR; // 披甲战象 肉搏护甲
extern I32 ARMORED_ELEPHANT_PIERCE_ARMOR; // 披甲战象 投射护甲
extern Double ARMORED_ELEPHANT_ATTACK_INTERVAL; // 披甲战象 攻击间距
extern I32 ARMORED_ELEPHANT_ATTACK; // 披甲战象 攻击力

// --- 攻城武器 ---
extern I32 STONE_THROWER_COST_FOOD; // 投石器 食物成本
extern I32 STONE_THROWER_COST_WOOD; // 投石器 木材成本
extern I32 STONE_THROWER_COST_GOLD; // 投石器 黄金成本
extern I32 STONE_THROWER_COST_STONE; // 投石器 石材成本
extern I32 STONE_THROWER_TRAIN_TIME; // 投石器 生产时间
extern I32 STONE_THROWER_HP; // 投石器 生命值
extern Double STONE_THROWER_SPEED; // 投石器 移动速度
extern I32 STONE_THROWER_LINE_OF_SIGHT; // 投石器 视野
extern I32 STONE_THROWER_HAND_ARMOR; // 投石器 肉搏护甲
extern I32 STONE_THROWER_PIERCE_ARMOR; // 投石器 投射护甲
extern I32 STONE_THROWER_RANGE; // 投石器 攻击距离
extern Double STONE_THROWER_ATTACK_INTERVAL; // 投石器 攻击间距
extern I32 STONE_THROWER_ATTACK; // 投石器 攻击力

extern I32 CATAPULT_COST_FOOD; // 投石车 食物成本
extern I32 CATAPULT_COST_WOOD; // 投石车 木材成本
extern I32 CATAPULT_COST_GOLD; // 投石车 黄金成本
extern I32 CATAPULT_COST_STONE; // 投石车 石材成本
extern I32 CATAPULT_TRAIN_TIME; // 投石车 生产时间
extern I32 CATAPULT_HP; // 投石车 生命值
extern Double CATAPULT_SPEED; // 投石车 移动速度
extern I32 CATAPULT_LINE_OF_SIGHT; // 投石车 视野
extern I32 CATAPULT_HAND_ARMOR; // 投石车 肉搏护甲
extern I32 CATAPULT_PIERCE_ARMOR; // 投石车 投射护甲
extern I32 CATAPULT_RANGE; // 投石车 攻击距离
extern Double CATAPULT_ATTACK_INTERVAL; // 投石车 攻击间距
extern I32 CATAPULT_ATTACK; // 投石车 攻击力

extern I32 HEAVY_CATAPULT_COST_FOOD; // 重型投石车 食物成本
extern I32 HEAVY_CATAPULT_COST_WOOD; // 重型投石车 木材成本
extern I32 HEAVY_CATAPULT_COST_GOLD; // 重型投石车 黄金成本
extern I32 HEAVY_CATAPULT_COST_STONE; // 重型投石车 石材成本
extern I32 HEAVY_CATAPULT_TRAIN_TIME; // 重型投石车 生产时间
extern I32 HEAVY_CATAPULT_HP; // 重型投石车 生命值
extern Double HEAVY_CATAPULT_SPEED; // 重型投石车 移动速度
extern I32 HEAVY_CATAPULT_LINE_OF_SIGHT; // 重型投石车 视野
extern I32 HEAVY_CATAPULT_HAND_ARMOR; // 重型投石车 肉搏护甲
extern I32 HEAVY_CATAPULT_PIERCE_ARMOR; // 重型投石车 投射护甲
extern I32 HEAVY_CATAPULT_RANGE; // 重型投石车 攻击距离
extern Double HEAVY_CATAPULT_ATTACK_INTERVAL; // 重型投石车 攻击间距
extern I32 HEAVY_CATAPULT_ATTACK; // 重型投石车 攻击力

extern I32 BALLISTA_COST_FOOD; // 弩车 食物成本
extern I32 BALLISTA_COST_WOOD; // 弩车 木材成本
extern I32 BALLISTA_COST_GOLD; // 弩车 黄金成本
extern I32 BALLISTA_COST_STONE; // 弩车 石材成本
extern I32 BALLISTA_TRAIN_TIME; // 弩车 生产时间
extern I32 BALLISTA_HP; // 弩车 生命值
extern Double BALLISTA_SPEED; // 弩车 移动速度
extern I32 BALLISTA_LINE_OF_SIGHT; // 弩车 视野
extern I32 BALLISTA_HAND_ARMOR; // 弩车 肉搏护甲
extern I32 BALLISTA_PIERCE_ARMOR; // 弩车 投射护甲
extern I32 BALLISTA_RANGE; // 弩车 攻击距离
extern Double BALLISTA_ATTACK_INTERVAL; // 弩车 攻击间距
extern I32 BALLISTA_ATTACK; // 弩车 攻击力

extern I32 HELEPOLIS_COST_FOOD; // 巨型弩车 食物成本
extern I32 HELEPOLIS_COST_WOOD; // 巨型弩车 木材成本
extern I32 HELEPOLIS_COST_GOLD; // 巨型弩车 黄金成本
extern I32 HELEPOLIS_COST_STONE; // 巨型弩车 石材成本
extern I32 HELEPOLIS_TRAIN_TIME; // 巨型弩车 生产时间
extern I32 HELEPOLIS_HP; // 巨型弩车 生命值
extern Double HELEPOLIS_SPEED; // 巨型弩车 移动速度
extern I32 HELEPOLIS_LINE_OF_SIGHT; // 巨型弩车 视野
extern I32 HELEPOLIS_HAND_ARMOR; // 巨型弩车 肉搏护甲
extern I32 HELEPOLIS_PIERCE_ARMOR; // 巨型弩车 投射护甲
extern I32 HELEPOLIS_RANGE; // 巨型弩车 攻击距离
extern Double HELEPOLIS_ATTACK_INTERVAL; // 巨型弩车 攻击间距
extern I32 HELEPOLIS_ATTACK; // 巨型弩车 攻击力

// --- 船只系列 ---
extern I32 FISHING_BOAT_COST_FOOD; // 小渔船 食物成本
extern I32 FISHING_BOAT_COST_WOOD; // 小渔船 木材成本
extern I32 FISHING_BOAT_COST_GOLD; // 小渔船 黄金成本
extern I32 FISHING_BOAT_COST_STONE; // 小渔船 石材成本
extern I32 FISHING_BOAT_TRAIN_TIME; // 小渔船 生产时间
extern I32 FISHING_BOAT_HP; // 小渔船 生命值
extern Double FISHING_BOAT_SPEED; // 小渔船 移动速度
extern I32 FISHING_BOAT_LINE_OF_SIGHT; // 小渔船 视野
extern I32 FISHING_BOAT_HAND_ARMOR; // 小渔船 肉搏护甲
extern I32 FISHING_BOAT_PIERCE_ARMOR; // 小渔船 投射护甲

extern I32 FISHING_SHIP_COST_FOOD; // 渔船 食物成本
extern I32 FISHING_SHIP_COST_WOOD; // 渔船 木材成本
extern I32 FISHING_SHIP_COST_GOLD; // 渔船 黄金成本
extern I32 FISHING_SHIP_COST_STONE; // 渔船 石材成本
extern I32 FISHING_SHIP_TRAIN_TIME; // 渔船 生产时间
extern I32 FISHING_SHIP_HP; // 渔船 生命值
extern Double FISHING_SHIP_SPEED; // 渔船 移动速度
extern I32 FISHING_SHIP_LINE_OF_SIGHT; // 渔船 视野
extern I32 FISHING_SHIP_HAND_ARMOR; // 渔船 肉搏护甲
extern I32 FISHING_SHIP_PIERCE_ARMOR; // 渔船 投射护甲

extern I32 TRADE_BOAT_COST_FOOD; // 小商船 食物成本
extern I32 TRADE_BOAT_COST_WOOD; // 小商船 木材成本
extern I32 TRADE_BOAT_COST_GOLD; // 小商船 黄金成本
extern I32 TRADE_BOAT_COST_STONE; // 小商船 石材成本
extern I32 TRADE_BOAT_TRAIN_TIME; // 小商船 生产时间
extern I32 TRADE_BOAT_HP; // 小商船 生命值
extern Double TRADE_BOAT_SPEED; // 小商船 移动速度
extern I32 TRADE_BOAT_LINE_OF_SIGHT; // 小商船 视野
extern I32 TRADE_BOAT_HAND_ARMOR; // 小商船 肉搏护甲
extern I32 TRADE_BOAT_PIERCE_ARMOR; // 小商船 投射护甲

extern I32 MERCHANT_SHIP_COST_FOOD; // 商船 食物成本
extern I32 MERCHANT_SHIP_COST_WOOD; // 商船 木材成本
extern I32 MERCHANT_SHIP_COST_GOLD; // 商船 黄金成本
extern I32 MERCHANT_SHIP_COST_STONE; // 商船 石材成本
extern I32 MERCHANT_SHIP_TRAIN_TIME; // 商船 生产时间
extern I32 MERCHANT_SHIP_HP; // 商船 生命值
extern Double MERCHANT_SHIP_SPEED; // 商船 移动速度
extern I32 MERCHANT_SHIP_LINE_OF_SIGHT; // 商船 视野
extern I32 MERCHANT_SHIP_HAND_ARMOR; // 商船 肉搏护甲
extern I32 MERCHANT_SHIP_PIERCE_ARMOR; // 商船 投射护甲

extern I32 LIGHT_TRANSPORT_COST_FOOD; // 小型运输船 食物成本
extern I32 LIGHT_TRANSPORT_COST_WOOD; // 小型运输船 木材成本
extern I32 LIGHT_TRANSPORT_COST_GOLD; // 小型运输船 黄金成本
extern I32 LIGHT_TRANSPORT_COST_STONE; // 小型运输船 石材成本
extern I32 LIGHT_TRANSPORT_TRAIN_TIME; // 小型运输船 生产时间
extern I32 LIGHT_TRANSPORT_HP; // 小型运输船 生命值
extern Double LIGHT_TRANSPORT_SPEED; // 小型运输船 移动速度
extern I32 LIGHT_TRANSPORT_LINE_OF_SIGHT; // 小型运输船 视野
extern I32 LIGHT_TRANSPORT_HAND_ARMOR; // 小型运输船 肉搏护甲
extern I32 LIGHT_TRANSPORT_PIERCE_ARMOR; // 小型运输船 投射护甲

extern I32 HEAVY_TRANSPORT_COST_FOOD; // 大型运输船 食物成本
extern I32 HEAVY_TRANSPORT_COST_WOOD; // 大型运输船 木材成本
extern I32 HEAVY_TRANSPORT_COST_GOLD; // 大型运输船 黄金成本
extern I32 HEAVY_TRANSPORT_COST_STONE; // 大型运输船 石材成本
extern I32 HEAVY_TRANSPORT_TRAIN_TIME; // 大型运输船 生产时间
extern I32 HEAVY_TRANSPORT_HP; // 大型运输船 生命值
extern Double HEAVY_TRANSPORT_SPEED; // 大型运输船 移动速度
extern I32 HEAVY_TRANSPORT_LINE_OF_SIGHT; // 大型运输船 视野
extern I32 HEAVY_TRANSPORT_HAND_ARMOR; // 大型运输船 肉搏护甲
extern I32 HEAVY_TRANSPORT_PIERCE_ARMOR; // 大型运输船 投射护甲

extern I32 SCOUT_SHIP_COST_FOOD; // 侦察船 食物成本
extern I32 SCOUT_SHIP_COST_WOOD; // 侦察船 木材成本
extern I32 SCOUT_SHIP_COST_GOLD; // 侦察船 黄金成本
extern I32 SCOUT_SHIP_COST_STONE; // 侦察船 石材成本
extern I32 SCOUT_SHIP_TRAIN_TIME; // 侦察船 生产时间
extern I32 SCOUT_SHIP_HP; // 侦察船 生命值
extern Double SCOUT_SHIP_SPEED; // 侦察船 移动速度
extern I32 SCOUT_SHIP_LINE_OF_SIGHT; // 侦察船 视野
extern I32 SCOUT_SHIP_HAND_ARMOR; // 侦察船 肉搏护甲
extern I32 SCOUT_SHIP_PIERCE_ARMOR; // 侦察船 投射护甲
extern I32 SCOUT_SHIP_RANGE; // 侦察船 攻击距离
extern Double SCOUT_SHIP_ATTACK_INTERVAL; // 侦察船 攻击间距
extern I32 SCOUT_SHIP_ATTACK; // 侦察船 攻击力

extern I32 WAR_GALLEY_COST_FOOD; // 战船 食物成本
extern I32 WAR_GALLEY_COST_WOOD; // 战船 木材成本
extern I32 WAR_GALLEY_COST_GOLD; // 战船 黄金成本
extern I32 WAR_GALLEY_COST_STONE; // 战船 石材成本
extern I32 WAR_GALLEY_TRAIN_TIME; // 战船 生产时间
extern I32 WAR_GALLEY_HP; // 战船 生命值
extern Double WAR_GALLEY_SPEED; // 战船 移动速度
extern I32 WAR_GALLEY_LINE_OF_SIGHT; // 战船 视野
extern I32 WAR_GALLEY_HAND_ARMOR; // 战船 肉搏护甲
extern I32 WAR_GALLEY_PIERCE_ARMOR; // 战船 投射护甲
extern I32 WAR_GALLEY_RANGE; // 战船 攻击距离
extern Double WAR_GALLEY_ATTACK_INTERVAL; // 战船 攻击间距
extern I32 WAR_GALLEY_ATTACK; // 战船 攻击力

extern I32 FIRE_GALLEY_COST_FOOD; // 喷火船 食物成本
extern I32 FIRE_GALLEY_COST_WOOD; // 喷火船 木材成本
extern I32 FIRE_GALLEY_COST_GOLD; // 喷火船 黄金成本
extern I32 FIRE_GALLEY_COST_STONE; // 喷火船 石材成本
extern I32 FIRE_GALLEY_TRAIN_TIME; // 喷火船 生产时间
extern I32 FIRE_GALLEY_HP; // 喷火船 生命值
extern Double FIRE_GALLEY_SPEED; // 喷火船 移动速度
extern I32 FIRE_GALLEY_LINE_OF_SIGHT; // 喷火船 视野
extern I32 FIRE_GALLEY_HAND_ARMOR; // 喷火船 肉搏护甲
extern I32 FIRE_GALLEY_PIERCE_ARMOR; // 喷火船 投射护甲
extern I32 FIRE_GALLEY_RANGE; // 喷火船 攻击距离
extern Double FIRE_GALLEY_ATTACK_INTERVAL; // 喷火船 攻击间距
extern I32 FIRE_GALLEY_ATTACK; // 喷火船 攻击力

extern I32 TRIREME_COST_FOOD; // 三层桨座战船 食物成本
extern I32 TRIREME_COST_WOOD; // 三层桨座战船 木材成本
extern I32 TRIREME_COST_GOLD; // 三层桨座战船 黄金成本
extern I32 TRIREME_COST_STONE; // 三层桨座战船 石材成本
extern I32 TRIREME_TRAIN_TIME; // 三层桨座战船 生产时间
extern I32 TRIREME_HP; // 三层桨座战船 生命值
extern Double TRIREME_SPEED; // 三层桨座战船 移动速度
extern I32 TRIREME_LINE_OF_SIGHT; // 三层桨座战船 视野
extern I32 TRIREME_HAND_ARMOR; // 三层桨座战船 肉搏护甲
extern I32 TRIREME_PIERCE_ARMOR; // 三层桨座战船 投射护甲
extern I32 TRIREME_RANGE; // 三层桨座战船 攻击距离
extern Double TRIREME_ATTACK_INTERVAL; // 三层桨座战船 攻击间距
extern I32 TRIREME_ATTACK; // 三层桨座战船 攻击力

extern I32 CATAPULT_TRIREME_COST_FOOD; // 投石战船 食物成本
extern I32 CATAPULT_TRIREME_COST_WOOD; // 投石战船 木材成本
extern I32 CATAPULT_TRIREME_COST_GOLD; // 投石战船 黄金成本
extern I32 CATAPULT_TRIREME_COST_STONE; // 投石战船 石材成本
extern I32 CATAPULT_TRIREME_TRAIN_TIME; // 投石战船 生产时间
extern I32 CATAPULT_TRIREME_HP; // 投石战船 生命值
extern Double CATAPULT_TRIREME_SPEED; // 投石战船 移动速度
extern I32 CATAPULT_TRIREME_LINE_OF_SIGHT; // 投石战船 视野
extern I32 CATAPULT_TRIREME_HAND_ARMOR; // 投石战船 肉搏护甲
extern I32 CATAPULT_TRIREME_PIERCE_ARMOR; // 投石战船 投射护甲
extern I32 CATAPULT_TRIREME_RANGE; // 投石战船 攻击距离
extern Double CATAPULT_TRIREME_ATTACK_INTERVAL; // 投石战船 攻击间距
extern I32 CATAPULT_TRIREME_ATTACK; // 投石战船 攻击力

extern I32 JUGGERNAUT_COST_FOOD; // 巨型战舰 食物成本
extern I32 JUGGERNAUT_COST_WOOD; // 巨型战舰 木材成本
extern I32 JUGGERNAUT_COST_GOLD; // 巨型战舰 黄金成本
extern I32 JUGGERNAUT_COST_STONE; // 巨型战舰 石材成本
extern I32 JUGGERNAUT_TRAIN_TIME; // 巨型战舰 生产时间
extern I32 JUGGERNAUT_HP; // 巨型战舰 生命值
extern Double JUGGERNAUT_SPEED; // 巨型战舰 移动速度
extern I32 JUGGERNAUT_LINE_OF_SIGHT; // 巨型战舰 视野
extern I32 JUGGERNAUT_HAND_ARMOR; // 巨型战舰 肉搏护甲
extern I32 JUGGERNAUT_PIERCE_ARMOR; // 巨型战舰 投射护甲
extern I32 JUGGERNAUT_RANGE; // 巨型战舰 攻击距离
extern Double JUGGERNAUT_ATTACK_INTERVAL; // 巨型战舰 攻击间距
extern I32 JUGGERNAUT_ATTACK; // 巨型战舰 攻击力

// --- 防御塔 ---
extern I32 WATCH_TOWER_COST_FOOD; // 瞭望塔 食物成本
extern I32 WATCH_TOWER_COST_WOOD; // 瞭望塔 木材成本
extern I32 WATCH_TOWER_COST_GOLD; // 瞭望塔 黄金成本
extern I32 WATCH_TOWER_COST_STONE; // 瞭望塔 石材成本
extern I32 WATCH_TOWER_TRAIN_TIME; // 瞭望塔 生产时间
extern I32 WATCH_TOWER_HP; // 瞭望塔 生命值
extern I32 WATCH_TOWER_LINE_OF_SIGHT; // 瞭望塔 视野
extern I32 WATCH_TOWER_PIERCE_ARMOR; // 瞭望塔 投射护甲
extern I32 WATCH_TOWER_RANGE; // 瞭望塔 攻击距离
extern Double WATCH_TOWER_ATTACK_INTERVAL; // 瞭望塔 攻击间距
extern I32 WATCH_TOWER_ATTACK; // 瞭望塔 攻击力

extern I32 SENTRY_TOWER_COST_FOOD; // 哨塔 食物成本
extern I32 SENTRY_TOWER_COST_WOOD; // 哨塔 木材成本
extern I32 SENTRY_TOWER_COST_GOLD; // 哨塔 黄金成本
extern I32 SENTRY_TOWER_COST_STONE; // 哨塔 石材成本
extern I32 SENTRY_TOWER_TRAIN_TIME; // 哨塔 生产时间
extern I32 SENTRY_TOWER_HP; // 哨塔 生命值
extern I32 SENTRY_TOWER_LINE_OF_SIGHT; // 哨塔 视野
extern I32 SENTRY_TOWER_PIERCE_ARMOR; // 哨塔 投射护甲
extern I32 SENTRY_TOWER_RANGE; // 哨塔 攻击距离
extern Double SENTRY_TOWER_ATTACK_INTERVAL; // 哨塔 攻击间距
extern I32 SENTRY_TOWER_ATTACK; // 哨塔 攻击力

extern I32 GUARD_TOWER_COST_FOOD; // 守卫塔 食物成本
extern I32 GUARD_TOWER_COST_WOOD; // 守卫塔 木材成本
extern I32 GUARD_TOWER_COST_GOLD; // 守卫塔 黄金成本
extern I32 GUARD_TOWER_COST_STONE; // 守卫塔 石材成本
extern I32 GUARD_TOWER_TRAIN_TIME; // 守卫塔 生产时间
extern I32 GUARD_TOWER_HP; // 守卫塔 生命值
extern I32 GUARD_TOWER_LINE_OF_SIGHT; // 守卫塔 视野
extern I32 GUARD_TOWER_PIERCE_ARMOR; // 守卫塔 投射护甲
extern I32 GUARD_TOWER_RANGE; // 守卫塔 攻击距离
extern Double GUARD_TOWER_ATTACK_INTERVAL; // 守卫塔 攻击间距
extern I32 GUARD_TOWER_ATTACK; // 守卫塔 攻击力

extern I32 BALLISTA_TOWER_COST_FOOD; // 弩炮塔 食物成本
extern I32 BALLISTA_TOWER_COST_WOOD; // 弩炮塔 木材成本
extern I32 BALLISTA_TOWER_COST_GOLD; // 弩炮塔 黄金成本
extern I32 BALLISTA_TOWER_COST_STONE; // 弩炮塔 石材成本
extern I32 BALLISTA_TOWER_TRAIN_TIME; // 弩炮塔 生产时间
extern I32 BALLISTA_TOWER_HP; // 弩炮塔 生命值
extern I32 BALLISTA_TOWER_LINE_OF_SIGHT; // 弩炮塔 视野
extern I32 BALLISTA_TOWER_PIERCE_ARMOR; // 弩炮塔 投射护甲
extern I32 BALLISTA_TOWER_RANGE; // 弩炮塔 攻击距离
extern Double BALLISTA_TOWER_ATTACK_INTERVAL; // 弩炮塔 攻击间距
extern I32 BALLISTA_TOWER_ATTACK; // 弩炮塔 攻击力

// ==========================================
// 4. 建筑属性 (Buildings)
// ==========================================

// --- 城镇中心 (Town Center) ---
extern I32 TOWN_CENTER_COST_FOOD; // 城镇中心 食物花费
extern I32 TOWN_CENTER_COST_WOOD; // 城镇中心 木材花费
extern I32 TOWN_CENTER_COST_GOLD; // 城镇中心 黄金花费
extern I32 TOWN_CENTER_COST_STONE; // 城镇中心 石材花费
extern I32 TOWN_CENTER_BUILD_TIME; // 城镇中心 建造时间
extern I32 TOWN_CENTER_HP; // 城镇中心 生命值

// --- 房屋 (House) ---
extern I32 HOUSE_COST_FOOD; // 房屋 食物花费
extern I32 HOUSE_COST_WOOD; // 房屋 木材花费
extern I32 HOUSE_COST_GOLD; // 房屋 黄金花费
extern I32 HOUSE_COST_STONE; // 房屋 石材花费
extern I32 HOUSE_BUILD_TIME; // 房屋 建造时间
extern I32 HOUSE_HP; // 房屋 生命值

// --- 兵营 (Barracks) ---
extern I32 BARRACKS_COST_FOOD; // 兵营 食物花费
extern I32 BARRACKS_COST_WOOD; // 兵营 木材花费
extern I32 BARRACKS_COST_GOLD; // 兵营 黄金花费
extern I32 BARRACKS_COST_STONE; // 兵营 石材花费
extern I32 BARRACKS_BUILD_TIME; // 兵营 建造时间
extern I32 BARRACKS_HP; // 兵营 生命值

// ==========================================
// 5. 函数声明
// ==========================================
void ParseUnitAttribute(const char* filepath);