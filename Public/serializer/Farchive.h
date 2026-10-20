#ifndef FARCHIVE_H
#define FARCHIVE_H

#include "Public/type/type.h"
#include<vector>
#include"string"
#include <cstring>

class FArchive
{
protected:
    std::vector<Byte>data;
    bool write;
    bool error;
    U32 bit_pos;
public:
    FArchive();
    template<class T>
    void Serialize(T&val);
    void Serialize(I8&val);
    void Serialize(I16&val);
    void Serialize(I32&val);
    void Serialize(I64&val);
    void Serialize(U8&val);
    void Serialize(U16&val);
    void Serialize(U32&val);
    void Serialize(U64&val);
    void Serialize(Float64&val);
    void Serialize(Float32&val);
    void Serialize(bool&val);
    void Serialize(std::string&val);
    virtual void Serialize(void*dst,U32 bits)=0;
    bool IsRead()const;
    bool IsWrite()const;
    // 长度越界或缓冲区不够时置位，之后的 Serialize 不再读写。
    bool IsError()const { return error; }
    virtual void* GetData();
    // 缓冲区里实际占用的字节数。有效 bit 不一定占满最后一个字节。
    U32 ByteSize()const { return static_cast<U32>(data.size()); }
    virtual void Clear()=0;
    // 写侧：已经写入的 bit 数。读侧：还剩多少 bit，出错后为 0。
    virtual U32 Bits()const=0;
};

template<class T>
void FArchive::Serialize(T &val)
{
    val.Serialize(this);
}

#endif // FARCHIVE_H
