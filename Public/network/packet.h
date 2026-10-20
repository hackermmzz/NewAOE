#pragma once

#include "Public/type/type.h"
#include "Public/network/conf.h"
/*
此结构体只用于显示逻辑布局，实际不为数据的真实布局
真实布局由序列化保证
序列化保证布局符合逻辑布局
*/

//正常数据包结构布局
struct Packet{
    //是否是请求连接的包(对服务器是请求连接，对客户端是告诉他需要连接)
    bool bConnectionRequest : 1;
    //关闭连接请求
    bool bCloseConnection :1;
    //保活包,用于监测连接是否还存活
    bool bAliveKeep :1;
    //包所属连接的Cookie
    const char cookie[PACKET_COOKIE_SIZE];
    //包的编号
    U32 packInd;
    //整个包的大小（bit，不是字节)
    U16 packSize;
    //包含有Bunch的数量（比如对于单个连接可能发生合包）
    U8 bunchContain;
    struct{
        //大小（比特）
        U16 bunchSize;
        //属于的Bunch编号
        U32 bunchSeq;
        //在Bunch里面对应的索引
        U32 bunchInd;
        //数据
        Byte data[];
    };
};

//请求连接包布局
struct PacketEstablishPhaseOneClient{
    //请求当前的阶段
    U8 phase;
    //数据
    Byte data[];
};


struct PacketEstablishPhaseTwoClient{
    //请求当前的阶段
    U8 phase;
};


struct PacketEstablishPhaseThreeClient{
    //请求当前的阶段
    U8 phase;
    //Cookie
    Byte buf[PACKET_COOKIE_SIZE];
};

struct PacketEstablishPhaseServer{
    //是否ok进入下一阶段
    bool ok:1;
    //当前服务器维护的阶段
    U8 phase;
    //其他数据
    U16 size;
    //数据
    Byte buf[];
};

//关闭连接包布局
struct PacketCloseConnectionSender{
    bool notify:1;
    bool ack:1;
    Byte cookie[PACKET_COOKIE_SIZE];
};

struct PacketCloseConnectionPhaseReceiver{
    bool notify:1;
    bool ack:1;
    Byte cookie[PACKET_COOKIE_SIZE];
};


