#include "BloodHaver.h"
#include "Server/core/Public/var/Army.h"

Double BloodHaver::getMaxBlood(){
    return maxBlood;
}
Double BloodHaver::getBlood(){
    return currentBlood;
}
Double BloodHaver::getDefence(I32 attackType){
    return attackType == ATTACKTYPE_SHOOT ? defenceShoot : defenceClose;
}
bool BloodHaver::isDie(){
    if(currentBlood <= Double::Zero()){
        return true;
    }
    return false;
}
void BloodHaver::beHeal(Double heal){
    if(currentBlood<=Double::Zero())return;
    currentBlood += heal;
    if(currentBlood > maxBlood){
        currentBlood = maxBlood;
    }
}
void BloodHaver::beAttcked(Double damage){
    currentBlood -= damage;         ////注：这里不考虑防御力，防御力在计算伤害时已经算进去了
    if(currentBlood < Double::Zero()){
        currentBlood = Double::Zero();
    }
    notifyAttacked();   //被打断采集、拉仇恨、动物死亡后变可采集都挂在这个钩子上
}
Double BloodHaver::getDefenceCloseBasic(){
    return defenceClose;
}
Double BloodHaver::getDefenceShootBasic(){
    return defenceShoot;
}
//默认实现：没有科技加成的对象（动物、投掷物）加成恒为 0。
//Human / Building 覆写成「基础值取 ObejctBaseValue，加成 = 当前值 - 基础值」。
Double BloodHaver::getDefenceCloseAddition(){
    return Double::Zero();
}
Double BloodHaver::getDefenceShootAddition(){
    return Double::Zero();
}
void BloodHaver::notifyAttacked(){}

void BloodHaver::syncBloodAfterMaxChange(Double oldMaxBlood, Double oldCurrentBlood){
    if(oldMaxBlood <= Double::Zero()){
        //旧上限无效：说明这个对象构造时还没写过血量，按满血初始化
        currentBlood = maxBlood;
        return;
    }
    if(oldCurrentBlood <= Double::Zero()){
        //已经死掉的对象，不因为上限变化而复活
        currentBlood = Double::Zero();
        return;
    }
    //上限涨多少，当前血量就涨多少，缺口（旧上限-旧血量）保持不变
    currentBlood = oldCurrentBlood + (maxBlood - oldMaxBlood);
    if(currentBlood > maxBlood){
        currentBlood = maxBlood;
    }
    if(currentBlood < Double::Zero()){
        currentBlood = Double::Zero();
    }
}
