#pragma once



#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <list>
#include <string_view>
#include <thread>

#include "Public/profile/conf.h"
#include "Public/profile/profile_worker.h"
#include "Public/type/type.h"
#include "Public/util/util.h"


class ProfileLabel final {
public:
    // module must remain valid for the lifetime of this label. The profiling
    // macros satisfy this by constructing labels from static strings.
    // A callsite emits its LabelDefinition only once per process.
    explicit ProfileLabel(std::string_view module) noexcept : module_(module) {}

    template <std::size_t ModuleSize>
    explicit ProfileLabel(const char (&module)[ModuleSize]) noexcept
        : module_(module, ModuleSize - 1U) {}

    ProfileLabel(const ProfileLabel &) = delete;
    ProfileLabel &operator=(const ProfileLabel &) = delete;

private:
    friend class ProfileScope;

    // 0: unregistered, 1: registration in progress, >=2: LabelID + 1.
    std::atomic<U32> state_{0};
    std::string_view module_;
};

// A low-overhead RAII scope profiler.
//
// Static-label scopes perform two steady-clock reads, one cached atomic load,
// and bounded stack serialization. They perform no heap allocation and take
// no locks after first registration.
class ProfileScope final {
public:
    using Clock = std::chrono::steady_clock;

    enum class RecordType : U8 {
        Invalid = 0,
        DynamicEvent = 1,
        LabelDefinition = 2,
        ScopeEvent = 3,
    };

    enum RecordFlag : U8 {
        ModuleTruncated = 1U << 0U,
    };

    struct LabelView {
        U32 ID = 0;
        bool IsModuleTruncated = false;
        std::string_view Module;
    };

    struct EventView {
        U64 ID = 0; // Dynamic events only; static events use LabelID and timing.
        U64 StartNs = 0;
        U64 IntervalNs = 0;
        U32 LabelID = 0;
        bool IsModuleTruncated = false;
        std::string_view Module; // Dynamic events only.
    };

    // Static-label path. The label definition is emitted once, then every
    // scope event is a fixed 22-byte record: magic, type, startNs, intervalNs, labelID.
    FORCE_INLINE ProfileScope(ProfileLabel &label) noexcept;

    // Dynamic-label compatibility path. Strings are serialized into every
    // event and are therefore more expensive than ProfileLabel.
    FORCE_INLINE ProfileScope(const char *module) noexcept;
    FORCE_INLINE ProfileScope(std::string_view module) noexcept;

    ProfileScope(const ProfileScope &) = delete;
    ProfileScope &operator=(const ProfileScope &) = delete;
    ProfileScope(ProfileScope &&) = delete;
    ProfileScope &operator=(ProfileScope &&) = delete;

    void flush() noexcept;
    FORCE_INLINE void cancel() noexcept { completed_ = true; }
    [[nodiscard]] FORCE_INLINE bool isCompleted() const noexcept { return completed_; }

    FORCE_INLINE ~ProfileScope() noexcept;

    static RecordType GetRecordType(const Byte *data, U16 size) noexcept;
    static bool Decode(const Byte *data, U16 size, EventView &event) noexcept;
    static bool DecodeLabel(const Byte *data, U16 size, LabelView &label) noexcept;

private:
    void begin() noexcept;
    U32 resolveLabel(ProfileLabel &label) noexcept;
    void emitLabelDefinition(const ProfileLabel &label, U32 labelID) noexcept;
    void outputRecord(const Byte *data, U16 size) const noexcept;

    std::string_view module_;
    Clock::time_point timeBegin_{};
    U64 id_ = 0;
    U32 labelID_ = 0;
    bool completed_ = true;
};

namespace profile_detail {

inline std::atomic<U32> nextThreadID{1};
inline std::atomic<U32> nextLabelID{1};
inline thread_local U32 profileThreadID = 0;
inline thread_local U32 profileSequence = 0;

static_assert(ProfileThreadBufferSize >= MaxRecordSize,
              "thread buffer must hold one profile record");

FORCE_INLINE U32 currentThreadID() noexcept {
    if (profileThreadID == 0) {
        profileThreadID = nextThreadID.fetch_add(1, std::memory_order_relaxed);
    }
    return profileThreadID;
}

FORCE_INLINE U64 nextEventID(U32 threadID) noexcept {
    return (static_cast<U64>(threadID) << 32U) | static_cast<U64>(profileSequence++);
}

FORCE_INLINE std::string_view makeView(const char *text) noexcept {
    return text == nullptr ? std::string_view{} : std::string_view{text};
}

using PendingChunk = std::pair<std::array<Byte, ProfileThreadBufferSize>, U16>;

// Destroyed before the process-lifetime ProfileWorker, so the tail is submitted first.
struct ThreadProfileBuffer {
    std::list<PendingChunk> pending;
    Byte buf[ProfileThreadBufferSize];
    U32 bufFilled = 0;

    ~ThreadProfileBuffer() { flushToWorker(); }

    void flushToWorker() noexcept {
        if (bufFilled == 0 && pending.empty()) {
            return;
        }
        try {
            ProfileWorker &worker = ProfileWorker::instance();
            while (!pending.empty()) {
                const PendingChunk &head = pending.front();
                if (!worker.submit(head.first.data(), head.second)) {
                    worker.spill(head.first.data(), head.second);
                }
                pending.pop_front();
            }
            if (bufFilled != 0) {
                if (!worker.submit(buf, static_cast<U16>(bufFilled))) {
                    worker.spill(buf, bufFilled);
                }
                bufFilled = 0;
            }
        } catch (...) {
        }
    }
};

inline thread_local ThreadProfileBuffer threadProfileBuffer;

FORCE_INLINE void hangPending(std::list<PendingChunk> &lst, const Byte *data, U32 size) {
    lst.emplace_back();
    std::memcpy(lst.back().first.data(), data, size);
    lst.back().second = static_cast<U16>(size);
}

FORCE_INLINE void submitProfileChunk(const Byte *data, U32 size) noexcept {
    if (size > ProfileThreadBufferSize) {
        ProfileWorker::instance().submit(data, static_cast<U16>(size));
        return;
    }
    try {
        std::list<PendingChunk> &lst = threadProfileBuffer.pending;
        if (!lst.empty()) {
            PendingChunk &head = lst.front();
            if (ProfileWorker::instance().submit(head.first.data(), head.second)) {
                lst.pop_front();
            }
            hangPending(lst, data, size);
            return;
        }
        if (!ProfileWorker::instance().submit(data, static_cast<U16>(size))) {
            hangPending(lst, data, size);
        }
    } catch (...) {
    }
}

FORCE_INLINE void writeProfileBytes(const Byte *data, U32 size) noexcept {
    if (data == nullptr || size == 0) {
        return;
    }

    ThreadProfileBuffer &state = threadProfileBuffer;
    if (state.bufFilled != 0 &&
        (size > ProfileThreadBufferSize || state.bufFilled + size > ProfileThreadBufferSize)) {
        submitProfileChunk(state.buf, state.bufFilled);
        state.bufFilled = 0;
    }
    if (size > ProfileThreadBufferSize) {
        submitProfileChunk(data, size);
        return;
    }

    std::memcpy(state.buf + state.bufFilled, data, size);
    state.bufFilled += size;
}

template <typename Value>
FORCE_INLINE void append(Byte *buffer, U16 &offset, const Value &value) noexcept {
    std::memcpy(buffer + offset, &value, sizeof(Value));
    offset = static_cast<U16>(offset + sizeof(Value));
}

template <typename Value>
FORCE_INLINE bool read(const Byte *buffer, U16 size, U16 &offset, Value &value) noexcept {
    if (static_cast<U32>(offset) + sizeof(Value) > size) {
        return false;
    }
    std::memcpy(&value, buffer + offset, sizeof(Value));
    offset = static_cast<U16>(offset + sizeof(Value));
    return true;
}

} // namespace profile_detail

FORCE_INLINE void ProfileScope::begin() noexcept {
    if (labelID_ == 0) {
        id_ = profile_detail::nextEventID(profile_detail::currentThreadID());
    }
    timeBegin_ = Clock::now();
    completed_ = false;
}

FORCE_INLINE U32 ProfileScope::resolveLabel(ProfileLabel &label) noexcept {
    U32 state = label.state_.load(std::memory_order_acquire);
    if (state >= 2) {
        return state - 1U;
    }

    U32 expected = 0;
    if (label.state_.compare_exchange_strong(
            expected, 1U, std::memory_order_acq_rel, std::memory_order_acquire)) {
        const U32 labelID = profile_detail::nextLabelID.fetch_add(1, std::memory_order_relaxed);
        // Publish before emitting so a recursive entry of the same callsite
        // cannot deadlock on the registering state.
        label.state_.store(labelID + 1U, std::memory_order_release);
        emitLabelDefinition(label, labelID);
        return labelID;
    }

    do {
        std::this_thread::yield();
        state = label.state_.load(std::memory_order_acquire);
    } while (state == 1U);
    return state - 1U;
}

FORCE_INLINE void ProfileScope::emitLabelDefinition(const ProfileLabel &label, U32 labelID) noexcept {
    constexpr U16 payloadCapacity = MaxRecordSize - LabelDefinitionHeaderSize;
    const std::size_t moduleSize = std::min<std::size_t>(label.module_.size(), payloadCapacity);

    U8 flags = 0;
    if (moduleSize != label.module_.size()) {
        flags |= ModuleTruncated;
    }

    const U16 encodedModuleSize = static_cast<U16>(moduleSize);
    const U8 reserved = 0;
    std::array<Byte, MaxRecordSize> buffer;
    U16 offset = 0;

    profile_detail::append(buffer.data(), offset, WireMagic);
    profile_detail::append(buffer.data(), offset, RecordType::LabelDefinition);
    profile_detail::append(buffer.data(), offset, labelID);
    profile_detail::append(buffer.data(), offset, flags);
    profile_detail::append(buffer.data(), offset, reserved);
    profile_detail::append(buffer.data(), offset, encodedModuleSize);

    if (encodedModuleSize != 0) {
        std::memcpy(buffer.data() + offset, label.module_.data(), encodedModuleSize);
        offset = static_cast<U16>(offset + encodedModuleSize);
    }
    outputRecord(buffer.data(), offset);
}

FORCE_INLINE void ProfileScope::outputRecord(const Byte *data, U16 size) const noexcept {
    try {
        profile_detail::writeProfileBytes(data, size);
    } catch (...) {
        // Profiling must not change application control flow.
    }
}

FORCE_INLINE void ProfileScope::flush() noexcept {
    if (completed_) {
        return;
    }

    const Clock::time_point timeEnd = Clock::now();
    completed_ = true;
    const U64 startNs = static_cast<U64>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(timeBegin_.time_since_epoch()).count());
    const U64 intervalNs = static_cast<U64>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(timeEnd - timeBegin_).count());

    std::array<Byte, MaxRecordSize> buffer;
    U16 offset = 0;

    if (labelID_ != 0) {
        profile_detail::append(buffer.data(), offset, WireMagic);
        profile_detail::append(buffer.data(), offset, RecordType::ScopeEvent);
        profile_detail::append(buffer.data(), offset, startNs);
        profile_detail::append(buffer.data(), offset, intervalNs);
        profile_detail::append(buffer.data(), offset, labelID_);
        outputRecord(buffer.data(), offset);
        return;
    }

    profile_detail::append(buffer.data(), offset, WireMagic);
    profile_detail::append(buffer.data(), offset, RecordType::DynamicEvent);

    constexpr U16 payloadCapacity = MaxRecordSize - DynamicEventHeaderSize;
    const std::size_t moduleSize = std::min<std::size_t>(module_.size(), payloadCapacity);
    U8 flags = 0;
    if (moduleSize != module_.size()) {
        flags |= ModuleTruncated;
    }

    const U16 encodedModuleSize = static_cast<U16>(moduleSize);
    const U8 reserved = 0;
    profile_detail::append(buffer.data(), offset, flags);
    profile_detail::append(buffer.data(), offset, reserved);
    profile_detail::append(buffer.data(), offset, encodedModuleSize);
    profile_detail::append(buffer.data(), offset, id_);
    profile_detail::append(buffer.data(), offset, startNs);
    profile_detail::append(buffer.data(), offset, intervalNs);

    if (encodedModuleSize != 0) {
        std::memcpy(buffer.data() + offset, module_.data(), encodedModuleSize);
        offset = static_cast<U16>(offset + encodedModuleSize);
    }
    outputRecord(buffer.data(), offset);
}

FORCE_INLINE ProfileScope::ProfileScope(ProfileLabel &label) noexcept {
    labelID_ = resolveLabel(label);
    begin();
}

FORCE_INLINE ProfileScope::ProfileScope(const char *module) noexcept
    : ProfileScope(profile_detail::makeView(module)) {}

FORCE_INLINE ProfileScope::ProfileScope(std::string_view module) noexcept
    : module_(module) {
    begin();
}

FORCE_INLINE ProfileScope::~ProfileScope() noexcept {
    flush();
}

FORCE_INLINE ProfileScope::RecordType ProfileScope::GetRecordType(const Byte *data, U16 size) noexcept {
    if (data == nullptr || size < 2) {
        return RecordType::Invalid;
    }

    U16 offset = 0;
    U8 magic = 0;
    U8 type = 0;
    if (!profile_detail::read(data, size, offset, magic) ||
        !profile_detail::read(data, size, offset, type) || magic != WireMagic) {
        return RecordType::Invalid;
    }
    if (type < static_cast<U8>(RecordType::DynamicEvent) ||
        type > static_cast<U8>(RecordType::ScopeEvent)) {
        return RecordType::Invalid;
    }
    return static_cast<RecordType>(type);
}

FORCE_INLINE bool ProfileScope::Decode(const Byte *data, U16 size, EventView &event) noexcept {
    const RecordType recordType = GetRecordType(data, size);
    if (recordType != RecordType::DynamicEvent && recordType != RecordType::ScopeEvent) {
        return false;
    }

    U16 offset = 0;
    U8 magic = 0;
    RecordType type = RecordType::Invalid;
    EventView decoded;
    if (!profile_detail::read(data, size, offset, magic) ||
        !profile_detail::read(data, size, offset, type) || magic != WireMagic || type != recordType) {
        return false;
    }

    if (recordType == RecordType::ScopeEvent) {
        if (size != StaticEventSize ||
            !profile_detail::read(data, size, offset, decoded.StartNs) ||
            !profile_detail::read(data, size, offset, decoded.IntervalNs) ||
            !profile_detail::read(data, size, offset, decoded.LabelID) || offset != size) {
            return false;
        }
        event = decoded;
        return true;
    }

    U8 flags = 0;
    U8 reserved = 0;
    U16 moduleSize = 0;
    if (size < DynamicEventHeaderSize ||
        !profile_detail::read(data, size, offset, flags) ||
        !profile_detail::read(data, size, offset, reserved) ||
        !profile_detail::read(data, size, offset, moduleSize) ||
        !profile_detail::read(data, size, offset, decoded.ID) ||
        !profile_detail::read(data, size, offset, decoded.StartNs) ||
        !profile_detail::read(data, size, offset, decoded.IntervalNs) ||
        offset != DynamicEventHeaderSize ||
        static_cast<U32>(offset) + moduleSize != size) {
        return false;
    }
    decoded.IsModuleTruncated = (flags & ModuleTruncated) != 0;
    decoded.Module = std::string_view(reinterpret_cast<const char *>(data + offset), moduleSize);
    event = decoded;
    return true;
}

FORCE_INLINE bool ProfileScope::DecodeLabel(const Byte *data, U16 size, LabelView &label) noexcept {
    if (GetRecordType(data, size) != RecordType::LabelDefinition) {
        return false;
    }

    U16 offset = 0;
    U8 magic = 0;
    RecordType type = RecordType::Invalid;
    U8 flags = 0;
    U8 reserved = 0;
    U16 moduleSize = 0;
    LabelView decoded;
    if (!profile_detail::read(data, size, offset, magic) ||
        !profile_detail::read(data, size, offset, type) ||
        !profile_detail::read(data, size, offset, decoded.ID) ||
        !profile_detail::read(data, size, offset, flags) ||
        !profile_detail::read(data, size, offset, reserved) ||
        !profile_detail::read(data, size, offset, moduleSize) ||
        magic != WireMagic || type != RecordType::LabelDefinition ||
        offset != LabelDefinitionHeaderSize ||
        static_cast<U32>(offset) + moduleSize != size) {
        return false;
    }
    decoded.IsModuleTruncated = (flags & ModuleTruncated) != 0;
    decoded.Module = std::string_view(reinterpret_cast<const char *>(data + offset), moduleSize);
    label = decoded;
    return true;
}

#define AOE_PROFILE_JOIN_IMPL(left, right) left##right
#define AOE_PROFILE_JOIN(left, right) AOE_PROFILE_JOIN_IMPL(left, right)

#if defined(AOE_DISABLE_PROFILING)
#define AOE_PROFILE_SCOPE(module) ((void)0)
#define AOE_PROFILE_FUNCTION() ((void)0)
#else
#define AOE_PROFILE_SCOPE_IMPL(module, line)                                                      \
    static ::ProfileLabel AOE_PROFILE_JOIN(aoeProfileLabel_, line)((module));                     \
    ::ProfileScope AOE_PROFILE_JOIN(aoeProfileScope_, line)(                                     \
        AOE_PROFILE_JOIN(aoeProfileLabel_, line))
#define AOE_PROFILE_SCOPE(module) AOE_PROFILE_SCOPE_IMPL((module), __LINE__)

#define AOE_PROFILE_FUNCTION() AOE_PROFILE_SCOPE(__func__)
#endif
