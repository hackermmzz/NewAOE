#pragma once

/***********************************对象种类************************/
enum ObjectSort {
    SORT_BUILDING=0,
    SORT_STATICRES,
    SORT_FARMER,
    SORT_ANIMAL,
    SORT_MISSILE,
    SORT_BUILDING_RESOURCE,
    SORT_ARMY
};


/***********************************采集到的资源种类***********************/
enum ProductSort {
    PRODUCTSORT_WOOD=0,
    PRODUCTSORT_GRANARYFOOD,
    PRODUCTSORT_STONE,
    PRODUCTSORT_GOLD,
    PRODUCTSORT_STOCKFOOD
};

enum ResourceSort {
    RESOURCESORT_WOOD=0,
    RESOURCESORT_FOOD,
    RESOURCESORT_STONE,
    RESOURCESORT_GOLD
};