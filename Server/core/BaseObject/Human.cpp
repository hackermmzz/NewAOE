#include "Human.h"
#include "Server/core/Public/config/ObjectBaseValue.h"
#include "Server/core/Public/var/Army.h"

#include <utility>

Human::Human(I32 sort,IVec2 detailPos,WeakPtr<Development> playerScience){
    this->sort = sort;
    setPos(detailPos);
    this->playerScience = playerScience;
    transported = false;
}

bool Human::getTransported() const{return transported;}
void Human::setTransported(bool value){this->transported = value;}
I32 Human::getArmyClass() const{return -1;}
Double Human::getAttackBasic(){
    const std::pair<Double,Double> base =ObjectBaseValue::getBasicAttack(getSort(),getType());
    return attackType == ATTACKTYPE_SHOOT ? base.second : base.first;
}
Double Human::getAttackAdditon(){
    const Double current = attackType == ATTACKTYPE_SHOOT ? attackShoot : attackClose;
    return current - getAttackBasic();
}
Double Human::getDefenceCloseBasic(){return ObjectBaseValue::getBasicDefence(getSort(),getType()).first;}
Double Human::getDefenceShootBasic(){return ObjectBaseValue::getBasicDefence(getSort(),getType()).second;}
Double Human::getDefenceCloseAddition(){return defenceClose - getDefenceCloseBasic();}
Double Human::getDefenceShootAddition(){return defenceShoot - getDefenceShootBasic();}

// 构造时写进 BloodHaver / Attacker / MoveObject 的都是「基础值」，
// 科技一变这些缓存就失效了，这里统一按「基础值 × 比例加成 + 数值加成」重算一遍。
void Human::refresh(){
    /*SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){return;}
    const I32 objectSort = getSort();
    const I32 objectType = getType();
    const I32 armyClass = getArmyClass();

    //---- 血量 ----
    //上限涨多少、当前血量跟着涨多少，残血单位不会被科技直接奶满
    const Double oldMaxBlood = maxBlood;
    const Double oldCurrentBlood = currentBlood;
    maxBlood=ObjectBaseValue::getBasicBlood(objectSort,objectType)*science->get_rate_Blood(objectSort,objectType)+Double(science->get_addition_Blood(objectSort,objectType));
    syncBloodAfterMaxChange(oldMaxBlood,oldCurrentBlood);

    //---- 移动速度 ----
    speed = ObjectBaseValue::getBasicSpeed(objectSort,objectType)*science->get_rate_Move(objectSort,objectType);

    //---- 防御（first=近战，second=远程）----
    const std::pair<Double,Double> defenceBase =
        ObjectBaseValue::getBasicDefence(objectSort,objectType);
    defenceClose = defenceBase.first
                 * science->get_rate_Defence(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE)
                 + Double(science->get_addition_Defence(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE));
    defenceShoot = defenceBase.second
                 * science->get_rate_Defence(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT)
                 + Double(science->get_addition_Defence(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT));

    //---- 攻击（first=近战，second=远程）----
    const std::pair<Double,Double> attackBase =
        ObjectBaseValue::getBasicAttack(objectSort,objectType);
    //基础攻击为 0 表示这个单位本来就不会用这种方式攻击（祭司、渔船都没有近战
    //攻击，投石兵没有近战攻击），不能被科技「凭空加成」出来，否则会出现祭司
    //能砍人、投石兵贴身比远程还疼这种结果。
    if(attackBase.first > Double::Zero()){
        attackClose = attackBase.first
                    * science->get_rate_Attack(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE)
                    + Double(science->get_addition_Attack(objectSort,objectType,armyClass,ATTACKTYPE_CLOSE));
    }else{
        attackClose = Double::Zero();
    }
    if(attackBase.second > Double::Zero()){
        attackShoot = attackBase.second
                    * science->get_rate_Attack(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT)
                    + Double(science->get_addition_Attack(objectSort,objectType,armyClass,ATTACKTYPE_SHOOT));
    }else{
        attackShoot = Double::Zero();
    }

    //---- 攻击方式、范围伤害与攻击距离 ----
    const std::pair<I32,I32> rangeBase =
        ObjectBaseValue::getBasicAttackRange(objectSort,objectType);
    //有远程伤害的按远程攻击处理，否则近战
    attackType = attackBase.second > Double::Zero() ? ATTACKTYPE_SHOOT : ATTACKTYPE_CLOSE;
    rangeEffect = rangeBase.first;
    range = rangeBase.first > 0;
    minAttackRange = rangeBase.second
                   + science->get_addition_DisAttack(objectSort,objectType,armyClass,attackType);*/
    refreshBlood();
    refreshDefence();
    refreshSpeed();
    refreshAttack();
}
void Human::refreshBlood(){
    //科技为空时按「基础值」算，不能让单位停在 0 血：
    //否则一出生就被 isDie() 判死，后面的防御/速度/攻击也全都不刷新了。
    Double newmaxBlood=ObjectBaseValue::getBasicBlood(sort,type);
    SafePtr<Development> science = playerScience.lock();
    if(science != nullptr){
        newmaxBlood=newmaxBlood*science->get_rate_Blood(sort,type)+science->get_addition_Blood(sort,type);
    }
    //第一次刷新（构造时 maxBlood/currentBlood 都是 0）直接按满血初始化。
    //这里不能用 isDie() 拦截：新单位出生时血量就是 0，一拦就永远停在 0，
    //后面的 refreshDefence/Speed/Attack 也会跟着被拦掉。
    if(maxBlood<=Double::Zero()){
        maxBlood=newmaxBlood;
        currentBlood=newmaxBlood;
        return;
    }
    //已经被打死的对象不因上限变化而复活
    if(currentBlood<=Double::Zero()){
        maxBlood=newmaxBlood;
        return;
    }
    //上限涨多少、当前血量跟着涨多少，残血单位不会被科技直接奶满
    currentBlood+=newmaxBlood-maxBlood;
    maxBlood=newmaxBlood;
    if(currentBlood>maxBlood)currentBlood=maxBlood;
    if(currentBlood<Double::Zero())currentBlood=Double::Zero();
}
void Human::refreshDefence(){
    if(isDie()){return;}
    const std::pair<Double,Double> base=ObjectBaseValue::getBasicDefence(getSort(),getType());
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){
        //没有科技：直接取基础值
        defenceClose=base.first;
        defenceShoot=base.second;
        return;
    }
    const I32 armyClass=getArmyClass();
    defenceClose=base.first*science->get_rate_Defence(sort,type,armyClass,ATTACKTYPE_CLOSE)+science->get_addition_Defence(sort,type,armyClass,ATTACKTYPE_CLOSE);
    defenceShoot=base.second*science->get_rate_Defence(sort,type,armyClass,ATTACKTYPE_SHOOT)+science->get_addition_Defence(sort,type,armyClass,ATTACKTYPE_SHOOT);
}
void Human::refreshSpeed(){
    if(isDie()){return;}
    speed = ObjectBaseValue::getBasicSpeed(sort,type);
    SafePtr<Development> science = playerScience.lock();
    if(science == nullptr){return;}
    speed = speed*science->get_rate_Move(sort,type);
}
void Human::refreshAttack(){
    if(isDie()){return;}
    SafePtr<Development> science = playerScience.lock();
    const std::pair<Double,Double> attackBase =ObjectBaseValue::getBasicAttack(sort,type);
    I32 armyClass=getArmyClass();
    if(attackBase.first > Double::Zero()){
        attackClose = science==nullptr ? attackBase.first
                    : attackBase.first*science->get_rate_Attack(sort,type,armyClass,ATTACKTYPE_CLOSE)+ Double(science->get_addition_Attack(sort,type,armyClass,ATTACKTYPE_CLOSE));
    }else{
        attackClose = Double::Zero();
    }
    if(attackBase.second > Double::Zero()){
        attackShoot = science==nullptr ? attackBase.second
                    : attackBase.second * science->get_rate_Attack(sort,type,armyClass,ATTACKTYPE_SHOOT) + Double(science->get_addition_Attack(sort,type,armyClass,ATTACKTYPE_SHOOT));
    }else{
        attackShoot = Double::Zero();
    }
    const std::pair<I32,I32> rangeBase = ObjectBaseValue::getBasicAttackRange(sort,type);
    attackType = attackBase.second > Double::Zero() ? ATTACKTYPE_SHOOT : ATTACKTYPE_CLOSE;
    rangeEffect = rangeBase.first;
    range = rangeBase.first > 0;
    minAttackRange = rangeBase.second;
    if(science != nullptr){
        minAttackRange += science->get_addition_DisAttack(sort,type,armyClass,attackType);
        attackCircle=science->get_speed_attack(sort,type,armyClass)[1];
        attackInteval=science->get_speed_attack(sort,type,armyClass)[0];
    }
    //间隔至少 1 帧：内核若拿它做取模/除法，0 会直接崩
    if(attackInteval<=0){attackInteval=1;}
    if(attackCircle<0){attackCircle=0;}
}
