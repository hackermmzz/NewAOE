#include "Public/serializer/FarchiveIn.h"
#include "Public/serializer/bitcopy.hpp"

FArchiveIn::FArchiveIn()
{
    write=0;
    bit_pos=0;
    bit_limit=0;
    buf=nullptr;
}

FArchiveIn::FArchiveIn(const void* data0, U32 bits)
    : FArchiveIn(data0, bits, 0)
{
}

FArchiveIn::FArchiveIn(const void* data0, U32 bits, U32 from):FArchiveIn()
{
    if (bits == 0)
    {
        return;
    }
    if (data0 == nullptr || static_cast<U64>(from) + bits > 0xFFFFFFFFu)
    {
        error = true;
        return;
    }
    const U32 nbytes = static_cast<U32>((static_cast<U64>(bits) + 7u) >> 3);
    data.resize(nbytes);
    BitCopy(&data[0], 0, data0, from, bits);
    bit_limit = bits;
}

void FArchiveIn::SetBuffer(const void* source, U32 bits)
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

void* FArchiveIn::GetData()
{
    if (buf != nullptr)
    {
        return const_cast<void*>(buf);
    }
    return FArchive::GetData();
}

void FArchiveIn::Serialize(void* dst, U32 bits)
{
    if (bits == 0 || error)
    {
        return;
    }
    void* src = GetData();
    if (dst == nullptr || src == nullptr
        || static_cast<U64>(bit_pos) + bits > bit_limit)
    {
        error = true;
        return;
    }
    BitCopy(dst, 0, src, bit_pos, bits);
    bit_pos += bits;
}

void FArchiveIn::Clear()
{
    data.clear();
    buf=nullptr;
    bit_pos=0;
    bit_limit=0;
    error=0;
}

U32 FArchiveIn::Bits() const
{
    if (error || bit_pos >= bit_limit)
    {
        return 0;
    }
    return bit_limit - bit_pos;
}

U32 FArchiveIn::ByteSize() const
{
    return (Bits()+7)>>3;
}

void FArchiveIn::SetPos(U32 bit_pos0)
{
    bit_pos=bit_pos0;
}

U32 FArchiveIn::GetPos() const
{
    return bit_pos;
}

U32 FArchiveIn::BitsLeft() const
{
    return bit_limit-bit_pos;
}

bool FArchiveIn::IsEnd()const
{
    return bit_pos==bit_limit;
}
