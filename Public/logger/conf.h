#pragma once

#include "Public/type/type.h"

// 级别数值和 spdlog::level 一致。数值越大越严重。
namespace spdlog {
namespace level {
enum level_enum : U8 {
    trace = 0,    // 最详细，函数追踪、细节打点
    debug = 1,    // 调试信息，开发阶段看
    info = 2,     // 普通运行提示，默认级别
    warn = 3,     // 警告，潜在问题，但不影响主流程
    err = 4,      // 错误，功能出错，程序还能继续跑
    critical = 5, // 致命错误，大概率程序要挂
    off = 6       // 关闭日志
};
} // namespace level
} // namespace spdlog

static_assert(static_cast<U8>(spdlog::level::trace) == 0, "trace");
static_assert(static_cast<U8>(spdlog::level::debug) == 1, "debug");
static_assert(static_cast<U8>(spdlog::level::info) == 2, "info");
static_assert(static_cast<U8>(spdlog::level::warn) == 3, "warn");
static_assert(static_cast<U8>(spdlog::level::err) == 4, "err");
static_assert(static_cast<U8>(spdlog::level::critical) == 5, "critical");

// 运行时过滤门槛。比它更详细的调用直接返回。
constexpr spdlog::level::level_enum LogDefaultLevel = spdlog::level::info;
// 一条日志的最大字节数，含时间和换行。
constexpr U32 MaxLogLine = 1024;
// 每条线程的日志缓冲。下一条放不下时先提交。
constexpr U32 LogThreadBufferSize = 8192;
// submit 的一级环形槽数量，必须是 2 的幂。
constexpr I32 LogSubmitRingSize = 1024;
// 追加写入的日志文件。
constexpr char LogFile[] = "aoe.log";

static_assert(LogThreadBufferSize >= MaxLogLine, "thread buffer must hold one log line");
