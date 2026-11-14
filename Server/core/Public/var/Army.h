#pragma once

/*******************************************所有的兵种**********************/

enum ARMYTYPE{
    //军营生产
    AT_CLUBMAN = 0,         // 棍棒兵
    AT_AXEMAN,              // 刀斧兵

    AT_SHORT_SWORDSMAN,     // 短剑兵
    AT_BROAD_SWORDSMAN,     // 阔剑兵
    AT_LONG_SWORDSMAN,      // 长剑兵
    AT_LEGION,              // 军团步兵

    AT_SLINGER,             // 投石兵
    //靶场生产
    AT_BOWMAN,              // 弓箭兵

    AT_IMPROVED_BOWMAN,     // 强弓兵
    AT_COMPOSITE_BOWMAN,    // 复合弓兵

    AT_HORSE_ARCHER,        // 弓骑兵
    AT_HEAVY_HORSE_ARCHER,  // 重装弓骑兵
    
    AT_ELEPHANT_ARCHER,     // 象弓兵
    
    AT_CHARIOT_ARCHER,      // 战车弓箭手

    //马厩生产
    AT_SCOUT,               //侦察骑兵

    AT_CAVALRY,             // 骑兵
    AT_HEAVY_CAVALRY,       // 重骑兵
    AT_CATAPHRACT,          // 铁甲骑兵

    AT_CAMEL_RIDER,         // 骆驼骑兵
    
    AT_CHARIOT,             // 战车
    AT_SCYTHECHARIOT,       // 刀轮战车
    
    AT_WAR_ELEPHANT,        // 战象
    AT_ARMORED_ELEPHANT,    // 装甲战象
    //攻城武器厂生产
    AT_STONE_THROWER,       // 投石车
    AT_CATAPULT,            // 中型投石车
    AT_HEAVY_CATAPULT,      // 重型投石车

    AT_BALLISTA,            // 弩炮
    AT_HELEPOLIS,           // 重型弩炮
    //船坞生产
    AT_SCOUT_SHIP,          // 侦察船
    AT_WAR_GALLEY,          // 桨帆船
    AT_TRIREME,             // 三列桨帆船
    
    AT_CATAPULT_TRIREME,    // 无敌战船
    
    AT_FIRESAILBOAT,        //火帆船
    //学院生产
    AT_HOPLITE,             // 方阵兵
    AT_PHALANX,             // 十夫长
    AT_CENTURION,           // 百夫长
    //神庙生产
    AT_PRIEST,//祭司
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


