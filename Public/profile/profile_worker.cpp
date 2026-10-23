#include "Public/profile/profile_worker.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <atomic>
#include <deque>
#include <fstream>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace ipc = boost::interprocess;

namespace {

void releaseSubmitInfo(ProfileWorker::SubmitInfo *info) noexcept {
    info->~SubmitInfo();
    ::operator delete(info);
}

void readRing(const Byte *data, U32 capacity, U32 offset, Byte *dst, U32 size) {
    offset %= capacity;
    const U32 first = size < capacity - offset ? size : capacity - offset;
    std::memcpy(dst, data + offset, first);
    if (first < size) {
        std::memcpy(dst + first, data, size - first);
    }
}

void writeRing(Byte *data, U32 capacity, U32 offset, const Byte *src, U32 size) {
    offset %= capacity;
    const U32 first = size < capacity - offset ? size : capacity - offset;
    std::memcpy(data + offset, src, first);
    if (first < size) {
        std::memcpy(data, src + first, size - first);
    }
}

// One message is the chunk writeRecord already packed: U64 thread id, U32 payload size, payload.
U32 oldestMessageBytes(const Byte *data, U32 capacity, U32 head, U32 used) {
    if (used < 12 || capacity < 12) {
        return 0;
    }
    Byte prefix[12];
    readRing(data, capacity, head, prefix, 12);
    U32 payload = 0;
    std::memcpy(&payload, prefix + 8, sizeof(payload));
    const U64 total = 12ULL + static_cast<U64>(payload);
    if (total == 0 || total > used || total > capacity) {
        return 0;
    }
    return static_cast<U32>(total);
}

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

constexpr U32 ProfileWebSocketQueueCap = 64;
constexpr U32 ChunkHeaderBytes = 12;
constexpr U8 RecordDynamic = 1;
constexpr U8 RecordLabel = 2;
constexpr U8 RecordScope = 3;
constexpr U32 LabelTableCap = 1024U * 1024U;
constexpr U32 LabelFrameCap = 48U * 1024U;

bool labelIdExists(const std::vector<Byte> &records, U32 labelId) {
    U32 offset = 0;
    const U32 size = static_cast<U32>(records.size());
    while (offset + LabelDefinitionHeaderSize <= size) {
        U32 existing = 0;
        U16 moduleSize = 0;
        std::memcpy(&existing, records.data() + offset + 2, sizeof(existing));
        std::memcpy(&moduleSize, records.data() + offset + 8, sizeof(moduleSize));
        const U32 record = static_cast<U32>(LabelDefinitionHeaderSize) + moduleSize;
        if (offset + record > size) {
            return false;
        }
        if (existing == labelId) {
            return true;
        }
        offset += record;
    }
    return false;
}

// 标签只在调用点第一次执行时写进某个 chunk。后连上的客户端看不到它，模块名就对不上。
void rememberLabels(const Byte *chunk, U16 len, std::vector<Byte> &records) {
    if (chunk == nullptr || len < ChunkHeaderBytes || records.size() >= LabelTableCap) {
        return;
    }
    U32 payload = 0;
    std::memcpy(&payload, chunk + 8, sizeof(payload));
    if (payload > static_cast<U32>(len - ChunkHeaderBytes)) {
        return;
    }
    U32 offset = ChunkHeaderBytes;
    const U32 end = ChunkHeaderBytes + payload;
    while (offset + 2U <= end && records.size() < LabelTableCap) {
        if (chunk[offset] != WireMagic) {
            return;
        }
        const U8 type = chunk[offset + 1];
        if (type == RecordLabel) {
            if (offset + LabelDefinitionHeaderSize > end) {
                return;
            }
            U32 labelId = 0;
            U16 moduleSize = 0;
            std::memcpy(&labelId, chunk + offset + 2, sizeof(labelId));
            std::memcpy(&moduleSize, chunk + offset + 8, sizeof(moduleSize));
            const U32 record = static_cast<U32>(LabelDefinitionHeaderSize) + moduleSize;
            if (offset + record > end) {
                return;
            }
            if (!labelIdExists(records, labelId)) {
                records.insert(records.end(), chunk + offset, chunk + offset + record);
            }
            offset += record;
        } else if (type == RecordScope) {
            if (offset + StaticEventSize > end) {
                return;
            }
            offset += StaticEventSize;
        } else if (type == RecordDynamic) {
            if (offset + DynamicEventHeaderSize > end) {
                return;
            }
            U16 moduleSize = 0;
            std::memcpy(&moduleSize, chunk + offset + 4, sizeof(moduleSize));
            const U32 record = static_cast<U32>(DynamicEventHeaderSize) + moduleSize;
            if (offset + record > end) {
                return;
            }
            offset += record;
        } else {
            return;
        }
    }
}

// 明文 ws 会话。握手是异步的。握手完成后，只有 io 线程会 write。
class WsSession : public std::enable_shared_from_this<WsSession> {
public:
    explicit WsSession(tcp::socket socket, std::function<std::vector<Byte>()> labels)
        : ws_(std::move(socket)), labels_(std::move(labels)) {}

    void run() {
        ws_.binary(true);
        ws_.read_message_max(64U * 1024U);
        const auto self = shared_from_this();
        ws_.async_accept([self](const beast::error_code &ec) {
            if (ec) {
                self->failed_ = true;
                return;
            }
            // 先把已经见过的标签定义发给这个连接，再开始收实时 chunk。
            self->writeLabels();
            if (!self->failed_) {
                self->open_ = true;
            }
        });
    }

    void writeLabels() {
        if (!labels_) {
            return;
        }
        const std::vector<Byte> records = labels_();
        U32 offset = 0;
        const U32 total = static_cast<U32>(records.size());
        while (offset < total) {
            U32 cursor = offset;
            U32 bytes = 0;
            while (cursor + LabelDefinitionHeaderSize <= total) {
                U16 moduleSize = 0;
                std::memcpy(&moduleSize, records.data() + cursor + 8, sizeof(moduleSize));
                const U32 record = static_cast<U32>(LabelDefinitionHeaderSize) + moduleSize;
                if (cursor + record > total) {
                    return;
                }
                if (bytes > 0 && bytes + record > LabelFrameCap) {
                    break;
                }
                bytes += record;
                cursor += record;
            }
            if (bytes == 0) {
                return;
            }
            std::vector<Byte> frame(static_cast<std::size_t>(ChunkHeaderBytes) + bytes);
            const U64 threadId = 0;
            std::memcpy(frame.data(), &threadId, sizeof(threadId));
            std::memcpy(frame.data() + 8, &bytes, sizeof(bytes));
            std::memcpy(frame.data() + ChunkHeaderBytes, records.data() + offset, bytes);
            beast::error_code ec;
            ws_.write(net::buffer(frame), ec);
            if (ec) {
                failed_ = true;
                return;
            }
            offset = cursor;
        }
    }

    // 还在握手时返回 true，调用方保留这个连接。写失败返回 false，调用方删掉它。
    bool write(const std::vector<Byte> &data) {
        if (failed_) {
            return false;
        }
        if (!open_) {
            return true;
        }
        beast::error_code ec;
        ws_.write(net::buffer(data), ec);
        if (ec) {
            failed_ = true;
            open_ = false;
        }
        return !ec;
    }

    void close() {
        failed_ = true;
        open_ = false;
        beast::error_code ec;
        ws_.next_layer().close(ec);
    }

private:
    websocket::stream<tcp::socket> ws_;
    std::function<std::vector<Byte>()> labels_;
    bool open_ = false;
    bool failed_ = false;
};

} // namespace

struct ProfileWorker::Storage {
    struct Item {
        std::thread::id threadId{};
        U32 size = 0;
        std::unique_ptr<Byte[]> data;
    };

    std::mutex spillMutex;
    std::vector<Item> spilled;
    std::ofstream output;
    std::unique_ptr<ipc::shared_memory_object> sharedMemory;
    std::unique_ptr<ipc::mapped_region> sharedRegion;
    net::io_context wsContext{1};
    net::executor_work_guard<net::io_context::executor_type> wsWork{net::make_work_guard(wsContext)};
    tcp::acceptor wsAcceptor{wsContext};
    std::vector<std::shared_ptr<WsSession>> wsSessions;
    std::mutex wsMutex;
    std::deque<std::shared_ptr<const std::vector<Byte>>> wsPending;
    bool wsDraining = false;
    std::thread wsThread;
    bool wsRunning = false;
    bool wsStopping = false;
    std::mutex labelMutex;
    std::vector<Byte> labelRecords;
};

ProfileWorker::ProfileWorker() : storage_(std::make_unique<Storage>()) {}

ProfileWorker::~ProfileWorker() {
    try {
        drain();
        for (;;) {
            std::vector<Storage::Item> items;
            {
                std::lock_guard<std::mutex> lock(storage_->spillMutex);
                if (storage_->spilled.empty()) {
                    break;
                }
                items.swap(storage_->spilled);
            }
            for (Storage::Item &item : items) {
                writeRecord(item.threadId, item.data.get(), item.size);
            }
            drain();
        }
    } catch (...) {
    }

    try {
        if (storage_->output.is_open()) {
            storage_->output.flush();
        }
    } catch (...) {
    }

    try {
        stopWebSocket();
    } catch (...) {
    }
}

void ProfileWorker::spill(const void *buf, U32 buf_size) {
    if (buf == nullptr || buf_size == 0 || storage_ == nullptr) {
        return;
    }

    Storage::Item item;
    item.threadId = std::this_thread::get_id();
    item.size = buf_size;
    item.data = std::make_unique<Byte[]>(buf_size);
    std::memcpy(item.data.get(), buf, buf_size);

    std::lock_guard<std::mutex> lock(storage_->spillMutex);
    storage_->spilled.push_back(std::move(item));
}

ProfileWorker::SubmitInfo *ProfileWorker::getSubmitInfo() {
    constexpr I32 indexMask = ProfileSubmitRingSize - 1;
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

void ProfileWorker::drain() {
    constexpr I32 indexMask = ProfileSubmitRingSize - 1;
    for (;;) {
        const I32 head = head_.load(std::memory_order_relaxed);
        const I32 tail = tail_.load(std::memory_order_acquire);
        if (head == tail) {
            return;
        }

        // A failed publish leaves a permanent hole. Skip it so later records still flush.
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

void ProfileWorker::run() {
    SubmitInfo *info = nullptr;
    for (I32 i = 0; i < ProfileSubmitRingSize; ++i) {
        info = getSubmitInfo();
        if (info == nullptr) {
            return;
        }
        processInfo(info);
        releaseSubmitInfo(info);
    }
}

void ProfileWorker::processInfo(SubmitInfo *info) {
    if (info == nullptr) {
        return;
    }
    writeRecord(info->thread_id, info->data, info->buf_size);
}

void ProfileWorker::writeRecord(std::thread::id threadId, const Byte *data, U32 size) {
    if (storage_ == nullptr || data == nullptr || size == 0) {
        return;
    }

    U64 threadValue = 0;
    const U32 idBytes = sizeof(threadId) < sizeof(threadValue) ? static_cast<U32>(sizeof(threadId))
                                                                : static_cast<U32>(sizeof(threadValue));
    std::memcpy(&threadValue, &threadId, idBytes);

    const U32 recordBytes = static_cast<U32>(sizeof(threadValue) + sizeof(size) + size);
    if (recordBytes > 65535U) {
        return;
    }
    std::unique_ptr<Byte[]> record = std::make_unique<Byte[]>(recordBytes);
    U32 offset = 0;
    std::memcpy(record.get() + offset, &threadValue, sizeof(threadValue));
    offset += static_cast<U32>(sizeof(threadValue));
    std::memcpy(record.get() + offset, &size, sizeof(size));
    offset += static_cast<U32>(sizeof(size));
    std::memcpy(record.get() + offset, data, size);

    write(record.get(), static_cast<U16>(recordBytes));
}

void ProfileWorker::write(void *buf, U16 len) {
    if (storage_ == nullptr || buf == nullptr || len == 0) {
        return;
    }
    switch (ProfileWriteWay) {
    case PROFILE_WRITE_WAY::PROFILE_WRITE_TO_FILE:
        writeToFile(buf, len);
        break;
    case PROFILE_WRITE_WAY::PROFILE_WRITE_TO_MEMMORY:
        writeToMemory(buf, len);
        break;
    case PROFILE_WRITE_WAY::PROFILE_WRITE_TO_WEBSOCKET:
        writeToWebSocket(buf, len);
        break;
    default:
        break;
    }
}

void ProfileWorker::writeToFile(void *buf, U16 len) {
    std::ofstream &output = storage_->output;
    if (!output.is_open()) {
        output.open(ProfileTraceFile, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!output) {
            return;
        }
    }
    output.write(static_cast<const char *>(buf), static_cast<std::streamsize>(len));
    if (output) {
        writeCount += len;
    }
}

void ProfileWorker::writeToMemory(void *buf, U16 len) {
    if (!openSharedTrace()) {
        return;
    }
    auto *base = static_cast<Byte *>(storage_->sharedRegion->get_address());
    const U32 regionSize = static_cast<U32>(storage_->sharedRegion->get_size());
    constexpr U32 headerBytes = sizeof(U32) * 2;
    if (regionSize <= headerBytes || len > regionSize - headerBytes) {
        return;
    }
    const U32 capacity = regionSize - headerBytes;
    Byte *data = base + headerBytes;
    U32 head = 0;
    U32 used = 0;
    std::memcpy(&head, base, sizeof(head));
    std::memcpy(&used, base + sizeof(head), sizeof(used));
    if (head >= capacity || used > capacity) {
        head = 0;
        used = 0;
    }
    // 一条消息是 writeRecord 打好的整块 chunk。放不下就丢掉最旧的那条，直到这条能放下。
    while (used + static_cast<U32>(len) > capacity) {
        const U32 msg = oldestMessageBytes(data, capacity, head, used);
        if (msg == 0) {
            head = 0;
            used = 0;
            break;
        }
        head = (head + msg) % capacity;
        used -= msg;
    }
    writeRing(data, capacity, (head + used) % capacity, static_cast<const Byte *>(buf), len);
    used += len;
    // 数据写完后先发布 head，再发布 used。新数据可能盖住刚丢掉的字节。
    // 读端若只看到新 head 和旧 used，多出来的是尾部旧字节，不会把新消息读错位。
    std::atomic_thread_fence(std::memory_order_seq_cst);
    std::memcpy(base, &head, sizeof(head));
    std::atomic_thread_fence(std::memory_order_seq_cst);
    std::memcpy(base + sizeof(head), &used, sizeof(used));
    writeCount += len;
}

void ProfileWorker::writeToWebSocket(void *buf, U16 len) {
    if (!openWebSocket()) {
        return;
    }
    const auto *bytes = static_cast<const Byte *>(buf);
    {
        std::lock_guard<std::mutex> lock(storage_->labelMutex);
        rememberLabels(bytes, len, storage_->labelRecords);
    }
    auto payload = std::make_shared<const std::vector<Byte>>(bytes, bytes + len);
    bool start = false;
    {
        std::lock_guard<std::mutex> lock(storage_->wsMutex);
        while (storage_->wsPending.size() >= ProfileWebSocketQueueCap) {
            storage_->wsPending.pop_front();
        }
        storage_->wsPending.push_back(std::move(payload));
        if (!storage_->wsDraining) {
            storage_->wsDraining = true;
            start = true;
        }
    }
    if (start) {
        net::post(storage_->wsContext, [this] { drainWebSocket(); });
    }
    writeCount += len;
}

bool ProfileWorker::openWebSocket() {
    if (storage_ == nullptr) {
        return false;
    }
    if (storage_->wsRunning) {
        return true;
    }
    beast::error_code ec;
    const tcp::endpoint endpoint(tcp::v4(), ProfileWebSocketPort);
    tcp::acceptor &acceptor = storage_->wsAcceptor;
    acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        return false;
    }
    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    acceptor.bind(endpoint, ec);
    if (ec) {
        acceptor.close();
        return false;
    }
    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        acceptor.close();
        return false;
    }
    acceptWebSocket();
    storage_->wsThread = std::thread([this] { storage_->wsContext.run(); });
    storage_->wsRunning = true;
    return true;
}

void ProfileWorker::acceptWebSocket() {
    storage_->wsAcceptor.async_accept([this](const beast::error_code &ec, tcp::socket socket) {
        if (storage_->wsStopping) {
            return;
        }
        if (!ec) {
            auto session = std::make_shared<WsSession>(std::move(socket), [this] {
                std::lock_guard<std::mutex> lock(storage_->labelMutex);
                return storage_->labelRecords;
            });
            storage_->wsSessions.push_back(session);
            session->run();
        }
        if (!storage_->wsStopping && storage_->wsAcceptor.is_open()) {
            acceptWebSocket();
        }
    });
}

void ProfileWorker::drainWebSocket() {
    for (;;) {
        std::shared_ptr<const std::vector<Byte>> payload;
        {
            std::lock_guard<std::mutex> lock(storage_->wsMutex);
            if (storage_->wsPending.empty()) {
                storage_->wsDraining = false;
                return;
            }
            payload = storage_->wsPending.front();
            storage_->wsPending.pop_front();
        }
        if (storage_->wsStopping || payload == nullptr) {
            continue;
        }
        auto it = storage_->wsSessions.begin();
        while (it != storage_->wsSessions.end()) {
            if ((*it)->write(*payload)) {
                ++it;
            } else {
                it = storage_->wsSessions.erase(it);
            }
        }
    }
}

void ProfileWorker::stopWebSocket() {
    if (storage_ == nullptr || !storage_->wsRunning) {
        return;
    }
    std::promise<void> stopped;
    net::post(storage_->wsContext, [this, &stopped] {
        storage_->wsStopping = true;
        beast::error_code ec;
        storage_->wsAcceptor.close(ec);
        for (const std::shared_ptr<WsSession> &session : storage_->wsSessions) {
            session->close();
        }
        storage_->wsSessions.clear();
        storage_->wsWork.reset();
        stopped.set_value();
    });
    stopped.get_future().wait();
    if (storage_->wsThread.joinable()) {
        storage_->wsThread.join();
    }
    storage_->wsRunning = false;
}

bool ProfileWorker::openSharedTrace() {
    if (storage_ == nullptr || storage_->sharedRegion != nullptr) {
        return storage_ != nullptr && storage_->sharedRegion != nullptr;
    }
    try {
        const std::string name = ProfileSharedMemoryName(GetCurrentPid());
        ipc::shared_memory_object::remove(name.c_str());
        auto memory = std::make_unique<ipc::shared_memory_object>(
            ipc::create_only, name.c_str(), ipc::read_write);
        memory->truncate(ProfileSharedMemoryBytes);
        auto region = std::make_unique<ipc::mapped_region>(*memory, ipc::read_write);
        U32 header[2] = {0, 0};
        std::memcpy(region->get_address(), header, sizeof(header));
        storage_->sharedMemory = std::move(memory);
        storage_->sharedRegion = std::move(region);
        return true;
    } catch (...) {
        return false;
    }
}
