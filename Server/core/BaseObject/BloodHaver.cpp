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
//防御的科技加成由 update() 直接烘进 defenceClose/defenceShoot，
//这两个函数保留给"需要单独取加成值"的场景，默认返回 0，避免重复计算
Double BloodHaver::getDefenceCloseAddition(){
    return Double::Zero();
}
Double BloodHaver::getDefenceShootAddition(){
    return Double::Zero();
}
void BloodHaver::notifyAttacked(){}
