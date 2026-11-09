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
    currentBlood -= damage;
    if(currentBlood < Double::Zero()){
        currentBlood = Double::Zero();
    }
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
