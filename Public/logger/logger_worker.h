#pragma once

#include "Public/logger/conf.h"
#include "Public/util/util.h"
#include <atomic>
#include <cstring>
#include <memory>
#include <new>

// Queues flushed log buffers. run() writes them; the destructor flushes the rest.
class LoggerWorker final {
public:
    struct SubmitInfo {
        U32 buf_size;
        Byte data[1];
    };

    LoggerWorker();
    ~LoggerWorker();

    static LoggerWorker &instance();

    // Claims a ring slot first. Returns false when the ring is full.
    bool submit(const void *buf, U32 buf_size);
    // Keeps a chunk that did not fit in the ring. The destructor writes it after the ring.
    void spill(const void *buf, U32 buf_size);
    void run();
    void flush();

private:
    struct Storage;

    bool reserveSlot(I32 &slot);
    SubmitInfo *getSubmitInfo();
    void processInfo(SubmitInfo *info);
    void drain();
    void writeRecord(const Byte *data, U32 size);
    std::unique_ptr<Storage> storage_;
    std::atomic<SubmitInfo *> slots_[LogSubmitRingSize];
    std::atomic<I32> head_{0};
    std::atomic<I32> tail_{0};
    U64 writeCount{0};
};

static_assert(LogSubmitRingSize > 1 && (LogSubmitRingSize & (LogSubmitRingSize - 1)) == 0,
              "LogSubmitRingSize must be a power of two");

FORCE_INLINE LoggerWorker &LoggerWorker::instance() {
    static LoggerWorker worker;
    return worker;
}

FORCE_INLINE bool LoggerWorker::reserveSlot(I32 &slot) {
    constexpr I32 indexMask = LogSubmitRingSize - 1;
    I32 tail = tail_.load(std::memory_order_relaxed);
    for (;;) {
        const I32 head = head_.load(std::memory_order_acquire);
        const I32 next = (tail + 1) & indexMask;
        if (next == head) {
            return false;
        }
        if (tail_.compare_exchange_weak(tail, next, std::memory_order_acq_rel,
                                        std::memory_order_relaxed)) {
            slot = tail;
            return true;
        }
    }
}

FORCE_INLINE bool LoggerWorker::submit(const void *buf, U32 buf_size) {
    if (buf == nullptr || buf_size == 0) {
        return false;
    }

    I32 slot = 0;
    if (!reserveSlot(slot)) {
        return false;
    }

    try {
        void *memory = ::operator new(sizeof(SubmitInfo) + static_cast<std::size_t>(buf_size));
        auto *info = new (memory) SubmitInfo();
        info->buf_size = buf_size;
        std::memcpy(info->data, buf, buf_size);
        slots_[slot].store(info, std::memory_order_release);
        return true;
    } catch (...) {
        return false;
    }
}
