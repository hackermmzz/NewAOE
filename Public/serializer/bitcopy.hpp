#pragma once

#include "Public/type/type.h"
#include <cstring>

// 字节内 bit0 是最低位。只改写 [DestBit, DestBit + BitCount)，其余 bit 保持不变。
// Dest 与 Src 的 bit 区间不能重叠。
inline void BitCopy(
    void* Dest,      // 目标buffer起始地址
    U32 DestBit,    // 目标从第几个bit开始写（bit偏移，不是字节）
    const void* Src,       // 源buffer起始地址
    U32 SrcBit,     // 源从第几个bit开始读
    U32 BitCount    // 需要拷贝多少bit
    )
{
    if (BitCount == 0)
    {
        return;
    }

    Byte* D = static_cast<Byte*>(Dest) + (DestBit >> 3);
    const Byte* S = static_cast<const Byte*>(Src) + (SrcBit >> 3);
    U32 DBit = DestBit & 7u;
    U32 SBit = SrcBit & 7u;

    // 两边都落在字节边界上时，整字节直接搬，尾部不足 8 bit 再按掩码合并。
    if (DBit == 0 && SBit == 0)
    {
        const U32 Bytes = BitCount >> 3;
        if (Bytes != 0)
        {
            std::memcpy(D, S, Bytes);
        }
        const U32 Remain = BitCount & 7u;
        if (Remain != 0)
        {
            const Byte Mask = static_cast<Byte>((1u << Remain) - 1u);
            D[Bytes] = static_cast<Byte>((D[Bytes] & static_cast<Byte>(~Mask)) | (S[Bytes] & Mask));
        }
        return;
    }

    // 偏移相同：先补齐当前字节，后面同样按整字节搬。
    if (DBit == SBit)
    {
        const U32 HeadRoom = 8u - DBit;
        const U32 Head = HeadRoom < BitCount ? HeadRoom : BitCount;
        const Byte HeadMask = static_cast<Byte>(((1u << Head) - 1u) << DBit);
        *D = static_cast<Byte>((*D & static_cast<Byte>(~HeadMask)) | (*S & HeadMask));
        BitCount -= Head;
        if (BitCount == 0)
        {
            return;
        }
        ++D;
        ++S;

        const U32 Bytes = BitCount >> 3;
        if (Bytes != 0)
        {
            std::memcpy(D, S, Bytes);
        }
        const U32 Remain = BitCount & 7u;
        if (Remain != 0)
        {
            const Byte TailMask = static_cast<Byte>((1u << Remain) - 1u);
            D[Bytes] = static_cast<Byte>((D[Bytes] & static_cast<Byte>(~TailMask)) | (S[Bytes] & TailMask));
        }
        return;
    }

    // 偏移不同：先补齐目标首字节。首部最多循环两次，因为每次至少
    // 会让源或目标推进到下一个字节边界。
    while (DBit != 0 && BitCount != 0)
    {
        const U32 SrcAvail = 8u - SBit;
        const U32 DestRoom = 8u - DBit;
        U32 N = BitCount;
        if (N > SrcAvail)
        {
            N = SrcAvail;
        }
        if (N > DestRoom)
        {
            N = DestRoom;
        }

        const Byte SrcMask = static_cast<Byte>(((1u << N) - 1u) << SBit);
        const Byte Bits = static_cast<Byte>(((*S & SrcMask) >> SBit) << DBit);
        const Byte DestMask = static_cast<Byte>(((1u << N) - 1u) << DBit);
        *D = static_cast<Byte>((*D & static_cast<Byte>(~DestMask)) | Bits);

        BitCount -= N;
        SBit += N;
        DBit += N;
        if (SBit == 8u)
        {
            SBit = 0;
            ++S;
        }
        if (DBit == 8u)
        {
            DBit = 0;
            ++D;
        }
    }

    if (BitCount == 0)
    {
        return;
    }

    // 目标现在按字节对齐。源仍不对齐，因此相邻两个源字节组成一个
    // 16-bit 窗口，每次直接生成一个完整目标字节。相比逐段推进，热
    // 循环中不再需要掩码合并和源/目标边界分支。
    while (BitCount >= 8u)
    {
        const U16 Window = static_cast<U16>(S[0]) |
            static_cast<U16>(static_cast<U16>(S[1]) << 8u);
        *D = static_cast<Byte>(Window >> SBit);
        ++S;
        ++D;
        BitCount -= 8u;
    }

    // 尾部不足一个字节，只覆盖目标低 BitCount 位。仅当有效位确实跨
    // 过源字节边界时读取 S[1]，避免在输入末尾发生越界读取。
    if (BitCount != 0)
    {
        U16 Window = S[0];
        if (SBit + BitCount > 8u)
        {
            Window = static_cast<U16>(Window |
                static_cast<U16>(static_cast<U16>(S[1]) << 8u));
        }
        const Byte TailMask = static_cast<Byte>((1u << BitCount) - 1u);
        const Byte Bits = static_cast<Byte>((Window >> SBit) & TailMask);
        *D = static_cast<Byte>((*D & static_cast<Byte>(~TailMask)) | Bits);
    }
}
