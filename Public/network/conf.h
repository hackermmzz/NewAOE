#pragma once
#include "Public/type/type.h"

/*******************************定义network模块里面的常量***************************/
//packet的cookie的固定大小(默认16字节，为uuid的大小)
constexpr U8 PACKET_COOKIE_SIZE=16;
//单个Packet包最大大小限制（符合MTU大小限制）,最大1400字节
constexpr U16 PACKET_MAX_SIZE=1400;
//组装完毕的Chunk大小限制(65535比特)
constexpr U16 CHUNK_MAX_SIZE=65535;
//监听的ip类型
constexpr const char* SOCK_LISTEN_LOCAL="127.0.0.1";
constexpr const char* SOCK_LISTEN_GLOBAL="0.0.0.0";
//请求连接阶段的HELLO阶段发送的数据
constexpr const char* CONNECTION_HELLO_BUF="Wang Lihong will be your first father!";
//连接建立超时时间(s)
constexpr U16 CONNECTION_ESTABLISH_TIMEOUT=30;
//连接断开的超时时间(s)
constexpr U16 CONNECTION_TIMEOUT=30;
//连接关闭超时时间(s)
constexpr U16 CONNECTION_CLOSE_TIMEOUT=30;

//枚举类型，表示连接类型
enum DriverTypeEnum{
    DRIVER_NET,
    DRIVER_PROC,
};


//请求连接阶段类型
enum ConnectionRequestPhaseEnum{
    CONN_REQ_HELLO=0,
    CONN_REQ_CONN=1,
    CONN_REQ_OK=2,
};