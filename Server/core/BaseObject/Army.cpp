#include "Army.h"
#include "Server/core/Public/var/Army.h"

/*
	//I32 armyClass;//军队的类别，比如骑兵，近战兵，远战兵
    //WeakPtr<Development> playerScience;	//科技
	//bool transported;	//是否被运输
	Double attackClose;			//近战伤害
	Double attackShoot;			//远程伤害
	I32 attackType;				//攻击类型
	bool range;					//是否为范围伤害
	I32 rangeEffect;			//范围伤害影响的范围
	I32 minAttackRange;			//攻击范围
	I32 attackCircle;			//多少帧后真正发动攻击
	I32 attackCurrentTick;		//当前进行到多少帧了
	I32 attackInteval;			//攻击间隔
	Double maxBlood;	//最大血量
	Double currentBlood;//当前血量
	Double defenceClose;//近战防御
	Double defenceShoot;//远程防御
	Double 			speed;   			//移动速度
	stack<IVec2> 	path;    			//记录路径
	IVec2 			targetDetailPos;	//目的地细节坐标
	//I32 playerID;						//所属玩家的编号（如果属于公立资源，那么为-1）
	//I32 teamID;	 						//所属团队的编号
	//I32 sort;							//对象的大类型，比如农民，军队等
	//I32 type;							//对象具体类型(比如运输船、农民、投石兵等这些具体角色）
	//U64 SN;								//全局对象编号
	string name;						//对象名称
	//IVec2 detail_pos;					//细节坐标
	//IVec2 block_pos;					//区块坐标
	I32 vision;							//视野范围
	union{
		CrashBox_Rect crashBoxRect;		//矩形碰撞包围盒
		CrashBox_Circle crashBoxCircle; //圆形碰撞包围盒
	};
	//static U64 globalSNIndex;			//给全局对象分配SN
*/
/*******************************************所有的兵种**********************/

/*enum ARMYTYPE{
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
};*/

/****************************兵种之间的小类别************************/
/*enum BRANCH_TYPE {
    ARMY_INFANTRY=0,//步兵
    ARMY_ARCHER,//弓兵
    ARMY_RIDER,//骑兵
    ARMY_FLAMEN,//祭司
    ARMY_SIEGE,//攻城兵器
    ARMY_SHIP//船
};*/



Army::Army(IVec2 detailPos,I32 type, WeakPtr<Development> playerScience)
    : Human(SORT_ARMY, detailPos, playerScience)
{
    this->type = type;
    switch(type){
        case AT_CLUBMAN:
        case AT_SWORDSMAN:
        case AT_HOPLITE:
        case AT_BROADSWORDSMAN:
        case AT_IMPROVED:
        {
            this->type =ARMY_INFANTRY;
            break;
        }
        case AT_SLINGER:
        case AT_BOWMAN:
        case AT_CHARIOT_ARCHER:
        case AT_COMPOSITE_BOWMAN:
        {
            this->type =ARMY_ARCHER;
            break;
        }
        case AT_SCOUT:
        case AT_CAVALRY:
        case AT_CHARIOT:
        {
            this->type =ARMY_RIDER;
            break;
        }
        case AT_PRIEST:
        {
            this->type =ARMY_FLAMEN;
            break;
        }
        case AT_STONE_THROWER:
        {
            this->type =ARMY_SIEGE;
            break;
        }
        case AT_SHIP:
        {
            this->type =ARMY_SHIP;
            break;
        }
        default:break;

    }
    //尚未完成

}