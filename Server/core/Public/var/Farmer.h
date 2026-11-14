#pragma once

/*******************************农民类型包含的种类****************************/
enum FarmerType {
    FARMERTYPE_FARMER = 0,      //普通农民
    FARMERTYPE_SMALL_SAILING,   //小型渔船
    FARMERTYPE_BIG_SAILING,     //大型渔船
    FARMERTYPE_SMALL_TRADE_BOAT,//小型商船
    FARMERTYPE_BIG_TRADE_BOAT,  //大型商船
    FARMERTYPE_LIGHT_TRANSPORT, //小型运输船
    FARMERTYPE_HEAVY_TRANSPORT, //重型运输船
    FARMERTYPE_MAXNUM
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
