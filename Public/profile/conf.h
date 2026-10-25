#pragma once

#include "Public/type/type.h"
#include <cstdint>
#include <string>

/***************************性能追踪日志方式***************************/
enum PROFILE_WRITE_WAY{
    PROFILE_WRITE_TO_FILE=0,    // 追加到 trace.profile
    PROFILE_WRITE_TO_MEMMORY=1, //追加到共享内存
    PROFILE_WRITE_TO_WEBSOCKET=2, //通过websocket转发数据
};
//当前启动的方式
constexpr PROFILE_WRITE_WAY ProfileWriteWay=PROFILE_WRITE_WAY::PROFILE_WRITE_TO_WEBSOCKET;
// 明文 ws 监听端口。每个已连接的客户端都会收到同一条 chunk。
constexpr U16 ProfileWebSocketPort = 9765;
// 共享内存名是 aoe_trace_memeory_<pid>，pid 用 GetCurrentPid()。
inline std::string ProfileSharedMemoryName(std::uint64_t pid) {
    return std::string("aoe_trace_memeory_") + std::to_string(pid);
}
// 共享内存总字节数。开头 8 字节是 head 和 used，后面是环形数据。写满后丢掉最旧的一条。
constexpr U32 ProfileSharedMemoryBytes = 4U * 1024U * 1024U;

/********************************开关，是否开启性能追踪**************/
#ifndef ENABLE_PROFILE_TRACE
#define ENABLE_PROFILE_TRACE 1
#endif

#if ENABLE_PROFILE_TRACE
#define PROFILE_TRACE() AOE_PROFILE_FUNCTION()
#else
#define PROFILE_TRACE() ((void)0) // 空实现，编译器直接优化没了
#endif

/********************************************************************************************************** */

// 每条记录的起始字节。后面紧跟 1 字节种类。
constexpr U8 WireMagic = 0xA5U;
// 动态事件头大小：魔数、种类、flags、reserved、moduleSize、id、start、interval。
constexpr U16 DynamicEventHeaderSize = 30;
// 静态事件整条长度：魔数、种类、startNs、intervalNs、labelID。
constexpr U16 StaticEventSize = 22;
// 标签定义头大小：魔数、种类、labelID、flags、reserved、moduleSize。
constexpr U16 LabelDefinitionHeaderSize = 10;
// 单条记录的最大字节数。
constexpr U16 MaxRecordSize = 1024;
// 每条线程的记录缓冲。下一条记录放不下时先刷新缓冲。
constexpr U32 ProfileThreadBufferSize = 8192;
static_assert(ProfileThreadBufferSize <= 65535U, "thread buffer must fit in a U16 submit size");
// submit 的一级环形槽数量，必须是 2 的幂。槽满了才进入 list。
constexpr I32 ProfileSubmitRingSize = 1024;
// processInfo 追加写入的跟踪文件。
constexpr char ProfileTraceFile[] = "trace.profile";



