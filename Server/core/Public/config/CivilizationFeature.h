#pragma once

#include <array>
#include <string>
#include <utility>

#include "basicValue.h"

/*
基础数值的统一查询入口。

所有函数都做成 static：调用方不需要实例（Object::getName 里就是静态调用）。
按 (sort, type) 查表，sort 就是 ObjectSort，type 是各枚举的类型下标。

越界处理：非法 sort/type 一律退回对应表的第一行，不抛异常、不解引用空指针。
调用方本来就该保证参数合法，这里只是兜底。
*/
class GameValue{
public:

    //----------------------------- 开局资源 -----------------------------
    //木、食、石、金；文明特色会替换其中某一项
    static std::array<I32,4> getStartResource(I32 civilization){
        std::array<I32,4> tmp;
        for(I32 i=0;i<4;i++){
            tmp[i]=__GameBasicValue::BasicStartResource[i];
        }
        //特色文明的资源替换（按覆盖处理；若原意是"额外加上"，把 = 改成 +=）
        for(I32 i=0;i<2;i++){
            if(__GameBasicValue::FeatureStartResource[i].first==civilization){
                const std::pair<I32,I32>& feature=__GameBasicValue::FeatureStartResource[i].second;
                if(feature.first>=0&&feature.first<4){
                    tmp[feature.first]=feature.second;
                }
            }
        }
        return tmp;
    }

    //----------------------------- 基础数值 -----------------------------
    //最大血量
    static Double getBasicBlood(I32 sort,I32 type){
        return Double(value(sort,type).blood);
    }
    //视野（块）
    static I32 getBasicVision(I32 sort,I32 type){
        return value(sort,type).vision;
    }
    //移动速度（细节单位/帧）
    static Double getBasicSpeed(I32 sort,I32 type){
        return milliToDouble(value(sort,type).speedMilli);
    }
    //攻击力：first 近战，second 远程
    static std::pair<Double,Double> getBasicAttack(I32 sort,I32 type){
        const BasicUnitValue& v=value(sort,type);
        return std::make_pair(Double(v.atkClose),Double(v.atkShoot));
    }
    //防御力：first 近战，second 远程
    static std::pair<Double,Double> getBasicDefence(I32 sort,I32 type){
        const BasicUnitValue& v=value(sort,type);
        return std::make_pair(Double(v.defClose),Double(v.defShoot));
    }
    //攻击距离：first 近战接战距离，second 远程射程（都是细节单位，0 表示没有）
    static std::pair<Double,Double> getBasicAttackRange(I32 sort,I32 type){
        const BasicUnitValue& v=value(sort,type);
        const Double closeRange(v.atkClose>0?DISTANCE_ATTACK_CLOSE:0);
        return std::make_pair(closeRange,Double(v.rangeShoot));
    }
    //攻击间隔（帧）
    static I32 getAttackInterval(I32 sort,I32 type){
        return value(sort,type).interval;
    }
    //圆形碰撞盒半径（细节单位，0 表示应该用矩形盒）
    static I32 getBasicRadius(I32 sort,I32 type){
        return value(sort,type).radius;
    }

    //----------------------------- 资源与建筑 -----------------------------
    //可采集存量（动物尸体 / 树木 / 矿 / 农田）
    static Double getResourceBasicMaxCnt(I32 sort,I32 type){
        return Double(value(sort,type).maxCnt);
    }
    //腐烂速度：配置里没有，暂返回 0（不腐烂）
    static Double getResourceBasicDecayRate(I32 sort,I32 type){
        (void)sort;
        (void)type;
        return Double::Zero();
    }
    //建造耗时（秒）
    static I32 getBasicTimeBuild(I32 sort,I32 type){
        return value(sort,type).timeBuild;
    }
    //建筑地基 FOUNDATION_*
    static I32 getBasicFoundation(I32 sort,I32 type){
        return value(sort,type).foundation;
    }

    //----------------------------- 村民 -----------------------------
    //采集携带上限
    static Double getFarmerBasicCollectMax(I32 sort,I32 type){
        (void)sort;
        (void)type;
        return Double(FARMER_CARRY_LIMIT);
    }
    //采集速度（每帧采集量）
    static Double getFarmerBasicCollectSpeed(I32 sort,I32 type){
        (void)sort;
        (void)type;
        return milliToDouble(FARMER_GATHER_MILLI);
    }

    //----------------------------- 名字 -----------------------------
    //形如 "棍棒兵#12"。playerID/teamID 暂时不用，保留是为了不改调用点。
    static std::string getObjectName(I32 playerID,I32 teamID,I32 sort,I32 type,U64 SN){
        (void)playerID;
        (void)teamID;

        const char* name="对象";
        switch(sort){
        case SORT_BUILDING:
        case SORT_BUILDING_RESOURCE:
            name=typeName(__GameBasicValue::BuildingName,BUILDING_TYPE_MAXNUM,type);
            break;
        case SORT_STATICRES:
            name=typeName(__GameBasicValue::StaticResName,4,type);
            break;
        case SORT_FARMER:
            name=typeName(__GameBasicValue::FarmerName,3,type);
            break;
        case SORT_ANIMAL:
            name=typeName(__GameBasicValue::AnimalName,6,type);
            break;
        case SORT_ARMY:
            name=typeName(__GameBasicValue::ArmyName,AT_ARMY_MAX_NUM,type);
            break;
        case SORT_MISSILE:
            name="投掷物";
            break;
        default:
            break;
        }
        return std::string(name)+"#"+std::to_string(SN);
    }

private:
    //按 (sort,type) 取表项；越界退回第一行
    static const BasicUnitValue& value(I32 sort,I32 type){
        if(sort<0||sort>=7){
            return __GameBasicValue::BuildingBasic[0];
        }
        const BasicUnitValue* table=__GameBasicValue::BasicTable[sort];
        const I32 size=__GameBasicValue::BasicTableSize[sort];
        if(type<0||type>=size){
            return table[0];
        }
        return table[type];
    }

    static const char* typeName(const char* const* names,I32 size,I32 type){
        if(type<0||type>=size){
            return "对象";
        }
        return names[type];
    }
};
