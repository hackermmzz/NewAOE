#pragma once

/*******************************************所有的兵种**********************/

enum ARMYTYPE{
    AT_CLUBMAN = 0,
    AT_SLINGER,
    AT_BOWMAN,
    AT_SCOUT,
    AT_SWORDSMAN,
    AT_IMPROVED,
    AT_CAVALRY,
    AT_SHIP,
    AT_STONE_THROWER,
    AT_PRIEST,
    AT_HOPLITE,
    AT_CHARIOT,             // 战车（四马战车）
    AT_CHARIOT_ARCHER,      // 战车弓箭手
    AT_BROADSWORDSMAN,      // 阔剑兵
    AT_COMPOSITE_BOWMAN,    // 复合弓兵
    AT_ARMY_MAX_NUM
};

/****************************兵种之间的小类别************************/
enum BRANCH_TYPE {
    ARMY_INFANTRY=0,//步兵
    ARMY_ARCHER,//弓兵
    ARMY_RIDER,//骑兵
    ARMY_FLAMEN,//祭司
    ARMY_SIEGE,//攻城兵器
    ARMY_SHIP//船
};

/***************************攻击方式**********************************/
/********** 攻击方式 **********/
enum AttackType {
    ATTACKTYPE_CLOSE,
    ATTACKTYPE_SHOOT,
    ATTACKTYPE_CHANGE,
};

