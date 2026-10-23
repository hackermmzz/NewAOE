#ifndef FARCHIVEIN_H
#define FARCHIVEIN_H

#include"Public/serializer/Farchive.h"

class FArchiveIn:public FArchive
{
public:
    using FArchive::Serialize;
    FArchiveIn();
    // 从 data 的 bit 0 起拷贝 bits 个 bit。
    FArchiveIn(const void* data, U32 bits);
    // 从 data 的第 from 个 bit 起拷贝 bits 个 bit。
    FArchiveIn(const void* data, U32 bits, U32 from);
    // 不拷贝，直接把 buf 里的 bits 个 bit 当作读源。
    void SetBuffer(const void* buf, U32 bits);
    virtual void* GetData();
    virtual void Serialize(void *dst, U32 bits);
    virtual void Clear();
    virtual U32 Bits()const;
    virtual U32 ByteSize()const;
    virtual void SetPos(U32 bit_pos);
    U32 GetPos()const;
    bool IsEnd()const;
    U32 BitsLeft()const;
private:
    const void* buf;
    U32 bit_limit;
};

#endif // FARCHIVEIN_H
