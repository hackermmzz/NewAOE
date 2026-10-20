#include "Public/serializer/Farchive.h"

FArchive::FArchive()
{
    write=0;
    error=0;
    bit_pos=0;
}

void FArchive::Serialize(I8 &val)
{
     Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(I16 &val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(U16 &val)
{
   Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(I64&val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(U8 &val)
{
     Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(U32&val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(U64&val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(I32&val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(Float64&val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(Float32&val)
{
    Serialize(static_cast<void*>(&val),sizeof(val)<<3);
}

void FArchive::Serialize(bool&val)
{
    Serialize(static_cast<void*>(&val),1);
}

void FArchive::Serialize(std::string&val)
{
    if (error)
    {
        return;
    }

    if (IsWrite())
    {
        const U64 count = val.size();
        if (count > (0xFFFFFFFFu >> 3))
        {
            error = true;
            return;
        }
        const U64 payload = count << 3;
        const U64 end = static_cast<U64>(bit_pos) + 32u + payload;
        if (end > 0xFFFFFFFFu)
        {
            error = true;
            return;
        }
        U32 siz = static_cast<U32>(count);
        Serialize(siz);
        if (siz == 0 || error)
        {
            return;
        }
        Serialize(static_cast<void*>(&val[0]), static_cast<U32>(payload));
        return;
    }

    U32 siz = 0;
    Serialize(siz);
    if (error)
    {
        return;
    }
    // 先确认剩余 bit 装得下正文，再扩容。长度来自输入，不能直接 resize。
    if ((static_cast<U64>(siz) << 3) > Bits())
    {
        error = true;
        return;
    }
    if (siz == 0)
    {
        val.clear();
        return;
    }
    val.resize(siz);
    Serialize(static_cast<void*>(&val[0]), siz << 3);
}

bool FArchive::IsRead() const
{
    return !IsWrite();
}

bool FArchive::IsWrite() const
{
    return write;
}


void* FArchive::GetData()
{
    return data.empty()?nullptr:&data[0];
}


