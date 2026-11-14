#pragma once

#include <string>
#include "Public/vec/vec.hpp"
#include "Server/core/BaseObject/CrashBox.h"
using namespace std;

class Object
{
protected:
	I32 playerID;						//所属玩家的编号（如果属于公立资源，那么为-1）
	I32 teamID;	 						//所属团队的编号
	//WeakPtr<Player> player;			//所属玩家//废弃，player内部拿不到自己的WeakPtr
	I32 sort;							//对象的大类型，比如农民，军队等
	I32 type;							//对象具体类型(比如运输船、农民、投石兵等这些具体角色）
	U64 SN;								//全局对象编号
	string name;						//对象名称
	IVec2 detail_pos;					//细节坐标
	IVec2 block_pos;					//区块坐标
	I32 vision;							//视野范围
	union{
		CrashBox_Rect crashBoxRect;		//矩形碰撞包围盒
		CrashBox_Circle crashBoxCircle; //圆形碰撞包围盒
	};
	static U64 globalSNIndex;			//给全局对象分配SN
public:
	Object();
	I32 getSort();
	I32 getType();
    //I32 getPlayerRepresent();
	//void setPlayerRepresent(I32 playerRepresent);
	IVec2 getBlockPos();
	IVec2 getDetailPos();
	string getName();
	I32 getVision();
  	I32 getglobalNum();
  	void setPos(const IVec2&pos);
	//void setplayer(WeakPtr<Player> player);
	//void getplayer(WeakPtr<Player> player);
	void setBelong(I32 playerID,I32 teamID);		//设置对象所属玩家和团队（只需要设一个的话，另一个填成负数就行）
	std::pair<I32,I32> getBelong();					//获取对象所属玩家和团队（first为playerID，second为teamID）
	virtual bool checkCrash(const Object*obj);
	virtual void tick();							//每帧处理结束都会调用这个函数
 };

class GameValue{
    private:
    //特色文明开局资源量
    static constexpr pair<I32,std::pair<I32,I32>> FeatureStartResource[2]={ {CIVILIZATION_SHANG, {RESOURCESORT_FOOD,160} }, {CIVILIZATION_PALMYRENEAN, {RESOURCESORT_FOOD,275} }};

    public:
    //获取开局初始资源
    static std::array<I32, 4> getStartResource(I32 civilization){
        array<I32, 4> tmp;
        for(I32 i=0;i<4;i++){
            tmp[i] = __GameBasicValue::BasicStartResource[i];
        }
        return tmp;
    }
    static Double getBasicBlood(I32 sort,I32 type){//这里不做检查，调用者自己检查，防止指针越界访问
        return Double(__GameBasicValue::BasicBlood[sort][type]);
    }
    //对象名字
    std::string getObjectName(I32 playerID,I32 teamID,I32 sort,I32 type,U64 SN){
        //return "Player："+std::to_string(plarerID)+"队伍："+std::to_string(teamID)+"类型"+std::to_string(sort)+"对象："+std::to_string(type)+"SN"+std::to_string(SN);
    }
    //基础资源最大值
    Double getResourceBasicMaxCnt(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicMaxCnt[sort]);
    }
    //基础腐烂速度
    Double getResourceBasicDecayRate(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicDecayRate);
    }
    //基础视野
    I32 getBasicVision(I32 sort,I32 type){
        return __GameBasicValue::BasicVision[sort][type];
    }
    //基础移动速度
    Double getBasicSpeed(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicSpeed[sort][type]);
    }
    //基础采集最大量
    Double getFarmerBasicCollectMax(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicCollectMax[sort][type]);
    }
    //基础采集速度
    Double getFarmerBasicCollectSpeed(I32 sort,I32 type){
        return Double(__GameBasicValue::BasicCollectSpeed[sort]);
    }
    //基础防御力
    std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){
        return {Double(__GameBasicValue::BasicDefenceClose[sort][type]),Double(__GameBasicValue::BasicDefenceShoot[sort][type])};
    }
    //基础攻击力
    std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){
        return {Double(__GameBasicValue::BasicAttackClose[sort][type]),Double(__GameBasicValue::BasicAttackShoot[sort][type])};
    }
    //基础攻击范围
    std::pair<Double,Double> getBasicAttackRange(I32 sort,I32 type){
        return {Double(__GameBasicValue::BasicAttackRangeClose[sort][type]),Double(__GameBasicValue::BasicAttackRangeShoot[sort][type])};
    }
    //基础攻击间隔
    I32 getAttackInterval(I32 sort,I32 type){
        return __GameBasicValue::BasicAttackInterval[sort][type];
    }
};

class __GameBasicValue{
    public:
    static constexpr I32 BasicStartResource[4] = {200,200,0,0}; //开局初始资源量
    static constexpr Double BasicBlood[][] = {0}; //基础血量
    static constexpr Double BasicMaxCnt[] = {0}; //基础资源最大值
    static constexpr Double BasicDecayRate = 0; //基础腐烂速度
    static constexpr I32 BasicVision[][] = {0}; //基础视野
    static constexpr Double BasicSpeed[][] = {0}; //基础移动速度
    static constexpr Double BasicCollectMax[][] = {0}; //基础采集最大量
    static constexpr Double BasicCollectSpeed[] = {0}; //基础采集速度
    static constexpr Double BasicDefenceClose[][] = {0}; //基础近战防御力
    static constexpr Double BasicDefenceShoot[][] = {0}; //基础远程防御力
    static constexpr Double BasicAttackClose[][] = {0}; //基础近战攻击力
    static constexpr Double BasicAttackShoot[][] = {0}; //基础远程攻击力
    static constexpr Double BasicAttackRangeClose[][] = {0}; //基础近战攻击范围
    static constexpr Double BasicAttackRangeShoot[][] = {0}; //基础远程攻击范围
    static constexpr I32 BasicAttackInterval[][] = {0}; //基础攻击间隔（帧）
};