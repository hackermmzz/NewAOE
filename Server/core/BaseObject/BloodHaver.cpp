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
}
void BloodHaver::beAttcked(Double damage){
    currentBlood -= damage;
}
Double BloodHaver::getDefenceCloseBasic(){
    return defenceShoot;
}
Double BloodHaver::getDefenceShootBasic(){
    return defenceClose;
}
Double BloodHaver::getDefenceCloseAddition(){
    //等待Development完成
}
Double BloodHaver::getDefenceShootAddition(){
    //等待Development完成
}
void BloodHaver::notifyAttacked(){}