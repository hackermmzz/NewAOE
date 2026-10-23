#pragma once

#include "Public/type/type.h"
#include "Public/network/conf.h"
#include "Public/network/bunch.h"
/*
此结构体只用于显示逻辑布局，实际不为数据的真实布局
真实布局由序列化保证
序列化保证布局符合逻辑布局
*/

struct PacketNotificationHeader{
    //发送端见到的最新的包的编号
    U64 ackedPacketID;
    //位图起始锚点
    U64 ackedHistorySeq;
    //位图有效位个数
    U16 historyCount;
    //位图最多表示多少位
    I8 history[(PACKET_ACKED_HISTORY_MAX_BITS+7)>>3]; 
};

//正常数据包结构布局
struct Packet{
    //是否是请求连接的包(对服务器是请求连接，对客户端是告诉他需要连接)
    bool bConnectionRequest : 1;
    //包所属连接的Cookie
    const char cookie[PACKET_COOKIE_SIZE];
    //关闭连接请求
    bool bCloseConnection :1;
    //包的编号
    U64 packID;
    //包头
    PacketNotificationHeader header;
    //bunch数组
    Bunch bunch[1];
};

//请求连接包布局
struct PacketEstablishPhaseOneClient{
    //请求当前的阶段
    U8 phase;
    //数据
    Byte data[1];
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
    Byte buf[1];
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


