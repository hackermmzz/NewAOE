#include "Public/logger/logger_worker.h"

#include <fstream>
#include <mutex>
#include <utility>
#include <vector>

namespace {

void releaseSubmitInfo(LoggerWorker::SubmitInfo *info) noexcept {
    info->~SubmitInfo();
    ::operator delete(info);
}

} // namespace

struct LoggerWorker::Storage {
    struct Item {
        U32 size = 0;
        std::unique_ptr<Byte[]> data;
    };

    std::ofstream output;
    std::mutex spillMutex;
    std::mutex consumeMutex;
    std::vector<Item> spilled;
};

LoggerWorker::LoggerWorker() : storage_(std::make_unique<Storage>()) {}

LoggerWorker::~LoggerWorker() {
    try {
        flush();
    } catch (...) {
    }
}

void LoggerWorker::spill(const void *buf, U32 buf_size) {
    if (buf == nullptr || buf_size == 0 || storage_ == nullptr) {
        return;
    }

    Storage::Item item;
    item.size = buf_size;
    item.data = std::make_unique<Byte[]>(buf_size);
    std::memcpy(item.data.get(), buf, buf_size);

    std::lock_guard<std::mutex> lock(storage_->spillMutex);
    storage_->spilled.push_back(std::move(item));
}

LoggerWorker::SubmitInfo *LoggerWorker::getSubmitInfo() {
    constexpr I32 indexMask = LogSubmitRingSize - 1;
    const I32 head = head_.load(std::memory_order_relaxed);
    const I32 tail = tail_.load(std::memory_order_acquire);
    if (head == tail) {
        return nullptr;
    }

    // The producer claims the slot before publishing the pointer.
    SubmitInfo *info = slots_[head].load(std::memory_order_acquire);
    if (info == nullptr) {
        return nullptr;
    }

    slots_[head].store(nullptr, std::memory_order_relaxed);
    head_.store((head + 1) & indexMask, std::memory_order_release);
    return info;
}

void LoggerWorker::drain() {
    constexpr I32 indexMask = LogSubmitRingSize - 1;
    for (;;) {
        const I32 head = head_.load(std::memory_order_relaxed);
        const I32 tail = tail_.load(std::memory_order_acquire);
        if (head == tail) {
            return;
        }

        // A failed publish leaves a permanent hole. Skip it so later lines still flush.
        SubmitInfo *info = slots_[head].load(std::memory_order_acquire);
        if (info == nullptr) {
            for (I32 spin = 0; spin < 64; ++spin) {
                info = slots_[head].load(std::memory_order_acquire);
                if (info != nullptr) {
                    break;
                }
            }
        }
        if (info == nullptr) {
            head_.store((head + 1) & indexMask, std::memory_order_release);
            continue;
        }

        slots_[head].store(nullptr, std::memory_order_relaxed);
        head_.store((head + 1) & indexMask, std::memory_order_release);
        processInfo(info);
        releaseSubmitInfo(info);
    }
}

void LoggerWorker::run() {
    if (storage_ == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(storage_->consumeMutex);
    bool wrote = false;
    for (I32 i = 0; i < LogSubmitRingSize; ++i) {
        SubmitInfo *info = getSubmitInfo();
        if (info == nullptr) {
            break;
        }
        processInfo(info);
        releaseSubmitInfo(info);
        wrote = true;
    }
    if (wrote && storage_->output.is_open()) {
        storage_->output.flush();
    }
}

void LoggerWorker::flush() {
    if (storage_ == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(storage_->consumeMutex);
    drain();
    for (;;) {
        std::vector<Storage::Item> items;
        {
            std::lock_guard<std::mutex> spillLock(storage_->spillMutex);
            if (storage_->spilled.empty()) {
                break;
            }
            items.swap(storage_->spilled);
        }
        for (Storage::Item &item : items) {
            writeRecord(item.data.get(), item.size);
        }
        drain();
    }
    if (storage_->output.is_open()) {
        storage_->output.flush();
    }
}

void LoggerWorker::processInfo(SubmitInfo *info) {
    if (info == nullptr) {
        return;
    }
    writeRecord(info->data, info->buf_size);
}

void LoggerWorker::writeRecord(const Byte *data, U32 size) {
    if (storage_ == nullptr || data == nullptr || size == 0) {
        return;
    }

    std::ofstream &output = storage_->output;
    if (!output.is_open()) {
        output.open(LogFile, std::ios::binary | std::ios::app);
        if (!output) {
            return;
        }
    }

    output.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
    if (output) {
        writeCount += size;
    }
}
