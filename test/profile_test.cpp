#include "Public/profile/profile.h"

#include <array>
#include <cassert>
#include <cstring>
#include <string>
#include <thread>

namespace {

struct CapturedRecord {
    std::array<Byte, MaxRecordSize> data{};
    U16 size = 0;
};

std::array<CapturedRecord, 16> records{};
U32 recordCount = 0;

U16 measureRecord(const Byte *data, U32 available) {
    assert(available >= 2);
    const U16 view = static_cast<U16>(available > 65535U ? 65535U : available);
    U16 offset = 0;
    U8 magic = 0;
    U8 type = 0;
    assert(profile_detail::read(data, view, offset, magic));
    assert(profile_detail::read(data, view, offset, type));
    assert(magic == WireMagic);
    const auto recordType = static_cast<ProfileScope::RecordType>(type);
    if (recordType == ProfileScope::RecordType::ScopeEvent) {
        assert(available >= StaticEventSize);
        return StaticEventSize;
    }

    U8 flags = 0;
    U8 reserved = 0;
    U16 moduleSize = 0;
    if (recordType == ProfileScope::RecordType::LabelDefinition) {
        U32 labelID = 0;
        assert(profile_detail::read(data, view, offset, labelID));
        assert(profile_detail::read(data, view, offset, flags));
        assert(profile_detail::read(data, view, offset, reserved));
        assert(profile_detail::read(data, view, offset, moduleSize));
        const U32 total = static_cast<U32>(LabelDefinitionHeaderSize) + moduleSize;
        assert(total <= available && total <= 65535U);
        return static_cast<U16>(total);
    }

    assert(recordType == ProfileScope::RecordType::DynamicEvent);
    assert(profile_detail::read(data, view, offset, flags));
    assert(profile_detail::read(data, view, offset, reserved));
    assert(profile_detail::read(data, view, offset, moduleSize));
    const U32 total = static_cast<U32>(DynamicEventHeaderSize) + moduleSize;
    assert(offset + 8 + 8 + 8 == DynamicEventHeaderSize);
    assert(total <= available && total <= 65535U);
    return static_cast<U16>(total);
}

void captureRange(const Byte *data, U32 size) {
    U32 offset = 0;
    while (offset < size) {
        const U16 recordSize = measureRecord(data + offset, size - offset);
        assert(recordCount < records.size());
        assert(recordSize <= records[recordCount].data.size());
        std::memcpy(records[recordCount].data.data(), data + offset, recordSize);
        records[recordCount].size = recordSize;
        ++recordCount;
        offset += recordSize;
    }
}

void captureSince(U32 mark) {
    const profile_detail::ThreadProfileBuffer &state = profile_detail::threadProfileBuffer;
    assert(state.bufFilled >= mark);
    captureRange(state.buf + mark, state.bufFilled - mark);
}

} // namespace

int main() {
    // Direct construction remains a dynamic-label compatibility path.
    {
        const U32 mark = profile_detail::threadProfileBuffer.bufFilled;
        ProfileScope scope("receive packet");
        scope.flush();
        captureSince(mark);
        const U32 filled = profile_detail::threadProfileBuffer.bufFilled;
        scope.flush();
        assert(profile_detail::threadProfileBuffer.bufFilled == filled);
    }
    assert(recordCount == 1);
    assert(ProfileScope::GetRecordType(records[0].data.data(), records[0].size) ==
           ProfileScope::RecordType::DynamicEvent);
    ProfileScope::EventView event;
    assert(ProfileScope::Decode(records[0].data.data(), records[0].size, event));
    assert(event.Module == "receive packet");
    assert(event.StartNs != 0);

    // A static macro emits one definition followed by fixed-size scope events.
    for (U32 index = 0; index < 2; ++index) {
        const U32 mark = profile_detail::threadProfileBuffer.bufFilled;
        {
            AOE_PROFILE_SCOPE("serializer.write");
        }
        captureSince(mark);
    }
    assert(recordCount == 4);
    ProfileScope::LabelView label;
    assert(ProfileScope::GetRecordType(records[1].data.data(), records[1].size) ==
           ProfileScope::RecordType::LabelDefinition);
    assert(ProfileScope::DecodeLabel(records[1].data.data(), records[1].size, label));
    assert(label.Module == "serializer.write");
    assert(label.ID != 0);

    ProfileScope::EventView firstStaticEvent;
    ProfileScope::EventView secondStaticEvent;
    assert(records[2].size == StaticEventSize);
    assert(records[3].size == StaticEventSize);
    assert(ProfileScope::Decode(records[2].data.data(), records[2].size, firstStaticEvent));
    assert(ProfileScope::Decode(records[3].data.data(), records[3].size, secondStaticEvent));
    assert(firstStaticEvent.LabelID == label.ID);
    assert(secondStaticEvent.LabelID == label.ID);

    // Dynamic records remain bounded and report truncation.
    const std::string longModule(2000, 'm');
    const U32 longMark = profile_detail::threadProfileBuffer.bufFilled;
    {
        ProfileScope scope{std::string_view(longModule)};
    }
    captureSince(longMark);
    assert(recordCount == 5);
    assert(records[4].size == MaxRecordSize);
    assert(ProfileScope::Decode(records[4].data.data(), records[4].size, event));
    assert(event.IsModuleTruncated);

    {
        const U32 mark = profile_detail::threadProfileBuffer.bufFilled;
        ProfileScope cancelled("unused");
        cancelled.cancel();
        assert(cancelled.isCompleted());
        assert(profile_detail::threadProfileBuffer.bufFilled == mark);
    }

    records[4].data[0] ^= 0xFFU;
    assert(!ProfileScope::Decode(records[4].data.data(), records[4].size, event));

    // Dynamic event IDs must remain unique across threads.
    CapturedRecord threadRecords[2];
    std::thread first([&] {
        const U32 mark = profile_detail::threadProfileBuffer.bufFilled;
        {
            ProfileScope scope("thread.first");
        }
        const profile_detail::ThreadProfileBuffer &state = profile_detail::threadProfileBuffer;
        assert(state.bufFilled >= mark);
        threadRecords[0].size = static_cast<U16>(state.bufFilled - mark);
        std::memcpy(threadRecords[0].data.data(), state.buf + mark, threadRecords[0].size);
    });
    std::thread second([&] {
        const U32 mark = profile_detail::threadProfileBuffer.bufFilled;
        {
            ProfileScope scope("thread.second");
        }
        const profile_detail::ThreadProfileBuffer &state = profile_detail::threadProfileBuffer;
        assert(state.bufFilled >= mark);
        threadRecords[1].size = static_cast<U16>(state.bufFilled - mark);
        std::memcpy(threadRecords[1].data.data(), state.buf + mark, threadRecords[1].size);
    });
    first.join();
    second.join();

    assert(ProfileScope::Decode(threadRecords[0].data.data(), threadRecords[0].size, firstStaticEvent));
    assert(ProfileScope::Decode(threadRecords[1].data.data(), threadRecords[1].size, secondStaticEvent));
    assert(firstStaticEvent.ID != secondStaticEvent.ID);
    return 0;
}
