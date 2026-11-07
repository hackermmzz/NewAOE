#pragma once

/*******************************************所有的兵种**********************/

enum ARMYTYPE{
    AT_CLUBMAN = 0,         // 棍棒兵
    AT_SLINGER,             // 投石兵
    AT_BOWMAN,              // 弓箭兵
    AT_SCOUT,               // 侦察骑兵
    AT_SWORDSMAN,           // 战斧兵或敌方近战单位
    AT_IMPROVED,            // 改进型单位或敌方单位
    AT_CAVALRY,             // 骑兵
    AT_SHIP,                // 战船
    AT_STONE_THROWER,       // 投石车
    AT_PRIEST,              // 祭司
    AT_HOPLITE,             // 方阵兵
    AT_CHARIOT,             // 战车（四马战车）
    AT_CHARIOT_ARCHER,      // 战车弓箭手
    AT_BROADSWORDSMAN,      // 阔剑兵
    AT_COMPOSITE_BOWMAN,    // 复合弓兵
    AT_ARMY_MAX_NUM
};

/****************************兵种之间的小类别************************/
enum BRANCH_TYPE {
    ARMY_INFANTRY=0,        //步兵
    ARMY_ARCHER,            //弓兵
    ARMY_RIDER,             //骑兵
    ARMY_FLAMEN,            //祭司
    ARMY_SIEGE,             //攻城兵器
    ARMY_SHIP               //船
};

/***************************攻击方式**********************************/
/********** 攻击方式 **********/
enum AttackType {
    ATTACKTYPE_CLOSE,       //近战
    ATTACKTYPE_SHOOT,       //远程
    ATTACKTYPE_CHANGE,      //可变
};

