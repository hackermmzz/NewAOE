#pragma once

/***********************************对象种类************************/
enum ObjectSort {
    SORT_BUILDING=0,                //建筑
    SORT_STATICRES,                 //静态资源
    SORT_FARMER,                    //农民
    SORT_ANIMAL,                    //动物
    SORT_MISSILE,                   //投掷物
    SORT_BUILDING_RESOURCE,         //资源型建筑（农田）
    SORT_ARMY,                      //军事人员
    SORT_MAXNUM
};


/***********************************采集到的资源种类***********************/
enum ProductSort {
    PRODUCTSORT_WOOD=0,             //木头
    PRODUCTSORT_GRANARYFOOD,        //谷仓食物（可以上交到谷仓或市镇中心的食物）
    PRODUCTSORT_STONE,              //石头
    PRODUCTSORT_GOLD,               //金矿
    PRODUCTSORT_STOCKFOOD           //仓库食物（各种肉类，可以上交到仓库或市镇中心的食物。如果是由渔船进行采集的话则只能上交到船坞。）
};

enum ResourceSort {
    RESOURCESORT_WOOD=0,            //木材
    RESOURCESORT_FOOD,              //食物
    RESOURCESORT_STONE,             //石矿
    RESOURCESORT_GOLD               //黄金
};