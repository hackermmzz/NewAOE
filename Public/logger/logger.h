#pragma once

#include "Public/logger/conf.h"
#include "Public/type/type.h"
#include "Public/util/util.h"

#if ENABLE_LOGGER

#include "Public/logger/logger_worker.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <list>
#include <string_view>
#include <utility>

// Text logger. Records at or above the current level go through a thread buffer
// into LoggerWorker, using the same ring path as the profiler.
class Logger final {
public:
    static void setLevel(spdlog::level::level_enum level) noexcept;
    [[nodiscard]] static spdlog::level::level_enum getLevel() noexcept;
    static void flush() noexcept;

    FORCE_INLINE static void log(spdlog::level::level_enum level, const char *message) noexcept;
    FORCE_INLINE static void log(spdlog::level::level_enum level, std::string_view message) noexcept;
    FORCE_INLINE static void trace(const char *message) noexcept;
    FORCE_INLINE static void debug(const char *message) noexcept;
    FORCE_INLINE static void info(const char *message) noexcept;
    FORCE_INLINE static void warn(const char *message) noexcept;
    FORCE_INLINE static void err(const char *message) noexcept;
    FORCE_INLINE static void critical(const char *message) noexcept;

private:
    static inline std::atomic<U8> level_{static_cast<U8>(LogDefaultLevel)};

    FORCE_INLINE static bool shouldLog(spdlog::level::level_enum level) noexcept;
    static void commit(spdlog::level::level_enum level, const char *message, U32 length) noexcept;
};

namespace log_detail {

using PendingChunk = std::pair<std::array<Byte, LogThreadBufferSize>, U32>;

struct ThreadLogBuffer {
    std::list<PendingChunk> pending;
    Byte buf[LogThreadBufferSize];
    U32 bufFilled = 0;

    ~ThreadLogBuffer() { flushToWorker(); }

    void flushToWorker() noexcept {
        if (bufFilled == 0 && pending.empty()) {
            return;
        }
        try {
            LoggerWorker &worker = LoggerWorker::instance();
            while (!pending.empty()) {
                const PendingChunk &head = pending.front();
                if (!worker.submit(head.first.data(), head.second)) {
                    worker.spill(head.first.data(), head.second);
                }
                pending.pop_front();
            }
            if (bufFilled != 0) {
                if (!worker.submit(buf, bufFilled)) {
                    worker.spill(buf, bufFilled);
                }
                bufFilled = 0;
            }
        } catch (...) {
        }
    }
};

inline thread_local ThreadLogBuffer threadLogBuffer;

FORCE_INLINE void hangPending(std::list<PendingChunk> &lst, const Byte *data, U32 size) {
    lst.emplace_back();
    std::memcpy(lst.back().first.data(), data, size);
    lst.back().second = size;
}

FORCE_INLINE void submitLogChunk(const Byte *data, U32 size) noexcept {
    if (size > LogThreadBufferSize) {
        LoggerWorker::instance().submit(data, size);
        return;
    }
    try {
        std::list<PendingChunk> &lst = threadLogBuffer.pending;
        if (!lst.empty()) {
            PendingChunk &head = lst.front();
            if (LoggerWorker::instance().submit(head.first.data(), head.second)) {
                lst.pop_front();
            }
            hangPending(lst, data, size);
            return;
        }
        if (!LoggerWorker::instance().submit(data, size)) {
            hangPending(lst, data, size);
        }
    } catch (...) {
    }
}

FORCE_INLINE void writeLogBytes(const Byte *data, U32 size) noexcept {
    if (data == nullptr || size == 0) {
        return;
    }

    ThreadLogBuffer &state = threadLogBuffer;
    if (state.bufFilled != 0 &&
        (size > LogThreadBufferSize || state.bufFilled + size > LogThreadBufferSize)) {
        submitLogChunk(state.buf, state.bufFilled);
        state.bufFilled = 0;
    }
    if (size > LogThreadBufferSize) {
        submitLogChunk(data, size);
        return;
    }

    std::memcpy(state.buf + state.bufFilled, data, size);
    state.bufFilled += size;
}

FORCE_INLINE const char *levelName(spdlog::level::level_enum level) noexcept {
    switch (level) {
    case spdlog::level::trace:
        return "trace";
    case spdlog::level::debug:
        return "debug";
    case spdlog::level::info:
        return "info";
    case spdlog::level::warn:
        return "warn";
    case spdlog::level::err:
        return "err";
    case spdlog::level::critical:
        return "critical";
    case spdlog::level::off:
        return "off";
    }
    return "off";
}

inline U32 formatPrefix(char *dest, U32 capacity, spdlog::level::level_enum level) noexcept {
    if (dest == nullptr || capacity < 2) {
        return 0;
    }

    const auto now = std::chrono::system_clock::now();
    const I64 millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    const std::time_t seconds = static_cast<std::time_t>(millis / 1000);
    const I32 ms = static_cast<I32>(millis % 1000);
    std::tm fields{};
#if defined(_WIN32)
    const bool ok = localtime_s(&fields, &seconds) == 0;
#else
    const bool ok = localtime_r(&seconds, &fields) != nullptr;
#endif
    I32 wrote = -1;
    if (ok) {
        wrote = static_cast<I32>(std::snprintf(
            dest, capacity, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] ",
            static_cast<I32>(fields.tm_year + 1900), static_cast<I32>(fields.tm_mon + 1),
            static_cast<I32>(fields.tm_mday), static_cast<I32>(fields.tm_hour),
            static_cast<I32>(fields.tm_min), static_cast<I32>(fields.tm_sec), ms, levelName(level)));
    }
    if (wrote < 0) {
        wrote = static_cast<I32>(std::snprintf(dest, capacity, "[%s] ", levelName(level)));
    }
    if (wrote < 0) {
        return 0;
    }
    if (static_cast<U32>(wrote) >= capacity) {
        return capacity - 1;
    }
    return static_cast<U32>(wrote);
}

} // namespace log_detail

inline void Logger::setLevel(spdlog::level::level_enum level) noexcept {
    level_.store(static_cast<U8>(level), std::memory_order_relaxed);
}

inline spdlog::level::level_enum Logger::getLevel() noexcept {
    return static_cast<spdlog::level::level_enum>(level_.load(std::memory_order_relaxed));
}

inline void Logger::flush() noexcept {
    try {
        log_detail::threadLogBuffer.flushToWorker();
        LoggerWorker::instance().flush();
    } catch (...) {
    }
}

FORCE_INLINE bool Logger::shouldLog(spdlog::level::level_enum level) noexcept {
    const U8 current = level_.load(std::memory_order_relaxed);
    return static_cast<U8>(level) >= current && level != spdlog::level::off;
}

inline void Logger::commit(spdlog::level::level_enum level, const char *message, U32 length) noexcept {
    try {
        Byte line[MaxLogLine];
        U32 used = log_detail::formatPrefix(reinterpret_cast<char *>(line), MaxLogLine, level);
        if (used >= MaxLogLine) {
            used = MaxLogLine - 1;
        }
        const U32 room = MaxLogLine - used - 1;
        const U32 copy = length < room ? length : room;
        if (copy != 0 && message != nullptr) {
            std::memcpy(line + used, message, copy);
            used += copy;
        }
        line[used++] = static_cast<Byte>('\n');
        log_detail::writeLogBytes(line, used);
        if (level >= spdlog::level::critical) {
            log_detail::threadLogBuffer.flushToWorker();
            LoggerWorker::instance().flush();
        }
    } catch (...) {
    }
}

FORCE_INLINE void Logger::log(spdlog::level::level_enum level, const char *message) noexcept {
    if (!shouldLog(level) || message == nullptr) {
        return;
    }
    const std::size_t length = std::strlen(message);
    const U32 capped = length > 0xffffffffu ? 0xffffffffu : static_cast<U32>(length);
    commit(level, message, capped);
}

FORCE_INLINE void Logger::log(spdlog::level::level_enum level, std::string_view message) noexcept {
    if (!shouldLog(level)) {
        return;
    }
    const U32 capped = message.size() > 0xffffffffu ? 0xffffffffu : static_cast<U32>(message.size());
    commit(level, message.data(), capped);
}

FORCE_INLINE void Logger::trace(const char *message) noexcept { log(spdlog::level::trace, message); }
FORCE_INLINE void Logger::debug(const char *message) noexcept { log(spdlog::level::debug, message); }
FORCE_INLINE void Logger::info(const char *message) noexcept { log(spdlog::level::info, message); }
FORCE_INLINE void Logger::warn(const char *message) noexcept { log(spdlog::level::warn, message); }
FORCE_INLINE void Logger::err(const char *message) noexcept { log(spdlog::level::err, message); }
FORCE_INLINE void Logger::critical(const char *message) noexcept {
    log(spdlog::level::critical, message);
}

//宏定义
#define LOGGER_TRACE() logger::trace(__func__)
#define LOGGER_DEBUG(msg) logger::debug(msg)
#define LOGGER_INFO(msg) logger::info(msg)
#define LOGGER_WARN(msg) logger::warn(msg)
#define LOGGER_ERR(msg) logger::err(msg)
#define LOGGER_CRITICAL(msg) logger::critical(msg)
#else
#define LOGGER_TRACE() ((void)0)
#define LOGGER_DEBUG(msg) ((void)0)
#define LOGGER_INFO(msg) ((void)0)
#define LOGGER_WARN(msg) ((void)0)
#define LOGGER_ERR(msg) ((void)0)
#define LOGGER_CRITICAL(msg) ((void)0)
#endif // ENABLE_LOGGER
