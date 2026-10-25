#pragma once
#include "Public/type/type.h"

/*******************************定义network模块里面的常量***************************/
//packet的cookie的固定大小(默认16字节，为uuid的大小)
constexpr U8 PACKET_COOKIE_SIZE=16;
//单个Packet包最大大小限制（符合MTU大小限制）,最大1400字节
constexpr U16 PACKET_MAX_SIZE=1400;
//组装完毕的Bunch大小限制(65535比特)
constexpr U32 BUNCH_MAX_SIZE=65535;
//监听的ip类型
constexpr const char* SOCK_LISTEN_LOCAL="127.0.0.1";
constexpr const char* SOCK_LISTEN_GLOBAL="0.0.0.0";
//请求连接阶段的HELLO阶段发送的数据
constexpr const char* CONNECTION_HELLO_BUF="Wang Lihong will be your first father!";
//连接建立超时时间(s)
constexpr U16 CONNECTION_ESTABLISH_TIMEOUT=30;
//连接断开的超时时间(s)
constexpr U16 CONNECTION_TIMEOUT=30;
//packet的ack历史位图最多表示个数
constexpr U16 PACKET_ACKED_HISTORY_MAX_BITS=256;
//connection最多可以保留在发送途中未确认丢失/收到 的包的数量
constexpr U16 PACKET_MAX_SEND_INSIGHT=PACKET_ACKED_HISTORY_MAX_BITS-(PACKET_ACKED_HISTORY_MAX_BITS>>3);
//keepAlive包发送间隔(ms)
constexpr U32 KEEP_ALIVE_SEND_INTERVAL=200;
//connection的待发送队列最大长度
constexpr U32 CONNECTION_MAX_SENDING_QUEUE_SIZE=1024;
//NetDriver单次可以接收的最大包数量
constexpr U16 DRIVER_RECEIVE_MAX_PACKAGE_NUM=1024;
//Connection接收方等待关闭时间，超时自动关闭(ms)
constexpr U16 CONNECTION_DRAINING_TIME=2000;
//不可靠Bunch分片队列长时间没有推进时，认为剩余分片已经丢失(ms)
constexpr U32 UNRELIABLE_BUNCH_RECV_STALL_TIMEOUT_MS=1000/3;
//不可靠Bunch接收队列中，单个缺口最多允许占当前Bunch索引跨度的比例分母
constexpr U8 UNRELIABLE_BUNCH_RECV_GAP_RATIO_DENOMINATOR=3;

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
