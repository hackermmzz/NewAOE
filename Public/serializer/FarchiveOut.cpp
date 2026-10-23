#include "Public/serializer/FarchiveOut.h"
#include "Public/serializer/bitcopy.hpp"

FArchiveOut::FArchiveOut()
{
    write=1;
    bit_pos=0;
    buf=nullptr;
    bit_limit=0;
}

void FArchiveOut::SetBuffer(void* source, U32 bits)
{
    bit_pos = 0;
    if (source == nullptr && bits != 0)
    {
        buf = nullptr;
        bit_limit = 0;
        error = true;
        return;
    }
    buf = source;
    bit_limit = bits;
    error = false;
}

void* FArchiveOut::GetData()
{
    if (buf != nullptr)
    {
        return buf;
    }
    return FArchive::GetData();
}

void FArchiveOut::Serialize(void *src, U32 bits)
{
    if (bits == 0 || error)
    {
        return;
    }
    if (src == nullptr)
    {
        error = true;
        return;
    }
    const U64 end = static_cast<U64>(bit_pos) + bits;
    if (end > 0xFFFFFFFFu)
    {
        error = true;
        return;
    }
    Byte* dst = nullptr;
    if (buf != nullptr)
    {
        if (end > bit_limit)
        {
            error = true;
            return;
        }
        dst = static_cast<Byte*>(buf);
    }
    else
    {
        const U32 targetByteNeed = static_cast<U32>((end + 7u) >> 3);
        data.resize(targetByteNeed);
        dst = &data[0];
    }
    BitCopy(dst, bit_pos, src, 0, bits);
    bit_pos = static_cast<U32>(end);
}

void FArchiveOut::Clear()
{
    data.clear();
    buf=nullptr;
    bit_pos=0;
    bit_limit=0;
    error=0;
}

U32 FArchiveOut::Bits() const
{
    return bit_pos;
}

U32 FArchiveOut::ByteSize() const
{
    if (buf != nullptr)
    {
        return static_cast<U32>((static_cast<U64>(bit_pos) + 7u) >> 3);
    }
    return static_cast<U32>(data.size());
}
