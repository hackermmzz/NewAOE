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
//默认实现：没有科技的对象（动物）加成恒为 0；
//Human/Building 会覆盖成"实际值 - 基础值"，供 UI 显示
Double BloodHaver::getDefenceCloseAddition(){
    return Double::Zero();
}
Double BloodHaver::getDefenceShootAddition(){
    return Double::Zero();
}
void BloodHaver::notifyAttacked(){}
