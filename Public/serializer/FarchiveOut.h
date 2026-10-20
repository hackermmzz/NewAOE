#ifndef FARCHIVEOUT_H
#define FARCHIVEOUT_H

#include"Public/serializer/Farchive.h"

class FArchiveOut:public FArchive
{
public:
    using FArchive::Serialize;
    FArchiveOut();
    virtual void Serialize(void*src,U32 bits);
    virtual void Clear();
    // 已经写入的 bit 数。整段缓冲区的字节数用 ByteSize()。
    virtual U32 Bits()const;
};

#endif // FARCHIVEOUT_H
