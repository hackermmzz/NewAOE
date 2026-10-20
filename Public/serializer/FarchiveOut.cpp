#include "Public/serializer/FarchiveOut.h"
#include "Public/serializer/bitcopy.hpp"

FArchiveOut::FArchiveOut()
{
    write=1;
    bit_pos=0;
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
    const U32 targetByteNeed = static_cast<U32>((end + 7u) >> 3);
    data.resize(targetByteNeed);
    BitCopy(&data[0], bit_pos, src, 0, bits);
    bit_pos = static_cast<U32>(end);
}

void FArchiveOut::Clear()
{
    data.clear();
    bit_pos=0;
    error=0;
}

U32 FArchiveOut::Bits() const
{
    return bit_pos;
}
