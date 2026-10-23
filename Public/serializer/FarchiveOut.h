#ifndef FARCHIVEOUT_H
#define FARCHIVEOUT_H

#include"Public/serializer/Farchive.h"

class FArchiveOut:public FArchive
{
public:
    using FArchive::Serialize;
    FArchiveOut();
    // 不拷贝。之后的写入直接进 buf 的前 bits 个 bit。
    void SetBuffer(void* buf, U32 bits);
    virtual void* GetData();
    virtual void Serialize(void*src,U32 bits);
    virtual void Clear();
    // 已经写入的 bit 数。实际占用的字节数用 ByteSize()。
    virtual U32 Bits()const;
    // 外部缓冲时是已写入的字节数，不是 SetBuffer 传入的容量。
    virtual U32 ByteSize()const;
private:
    void* buf;
    U32 bit_limit;
};

#endif // FARCHIVEOUT_H
