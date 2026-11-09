#pragma once

/*******************************农民类型包含的种类****************************/
enum FarmerType {
    FARMERTYPE_FARMER = 0,
    FARMERTYPE_WOOD_BOAT,   //运输船
    FARMERTYPE_SAILING      //渔船
};

/*********************************角色为农民时候的子角色**********************/
enum FarmerRole {
    FARMER_VILLAGER = 0,
    FARMER_LUMBER,
    FARMER_GATHERER,
    FARMER_MINER,
    FARMER_HUNTER,
    FARMER_FARMER,
    FARMER_WORKER,
    FARMER_FISHER
};
