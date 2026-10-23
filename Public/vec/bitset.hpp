#pragma once

#include "Public/type/type.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

// 固定 N 位，bit0 是最低位，和 std::bitset 一样。
// 只有一块字节数组，没有虚函数，对象可以直接 memcpy。
template<U32 N>
class BitSet {
    static_assert(N > 0, "BitSet至少要有1位");
    static_assert(N <= (0xffffffffu - 7u), "BitSet位数过大");

public:
    static constexpr U32 StorageBytes = (N + 7u) >> 3;

    class reference {
        friend class BitSet;
        BitSet* set_;
        U32 pos_;
        reference(BitSet& set, U32 pos) : set_(&set), pos_(pos) {}

    public:
        reference& operator=(bool value) {
            set_->set(pos_, value);
            return *this;
        }

        reference& operator=(const reference& other) {
            return *this = static_cast<bool>(other);
        }

        bool operator~() const {
            return !set_->test(pos_);
        }

        operator bool() const {
            return set_->test(pos_);
        }

        reference& flip() {
            set_->flip(pos_);
            return *this;
        }

        void operator&() const = delete;
    };

    constexpr BitSet(U64 value = 0) : storage{} {
        static_assert(std::is_trivially_copyable<BitSet>::value, "BitSet要能直接memcpy");
        static_assert(std::is_standard_layout<BitSet>::value, "BitSet要是标准布局");
        static_assert(!std::is_polymorphic<BitSet>::value, "BitSet不能有虚函数");
        static_assert(sizeof(BitSet) == StorageBytes, "BitSet不能有填充字节");

        constexpr U32 bytes = StorageBytes < 8u ? StorageBytes : 8u;
        for (U32 i = 0; i < bytes; i += 1) {
            storage[i] = static_cast<Byte>((value >> (8u * i)) & 0xffu);
        }
        ClearTail();
    }

    BitSet(const std::string& str,
           std::size_t pos = 0,
           std::size_t n = std::string::npos,
           char zero = '0',
           char one = '1')
        : storage{} {
        if (pos > str.size()) {
            throw std::out_of_range("BitSet");
        }
        const std::size_t remain = str.size() - pos;
        const std::size_t rlen = n < remain ? n : remain;
        for (std::size_t i = 0; i < rlen; i += 1) {
            const char c = str[pos + i];
            if (c != zero && c != one) {
                throw std::invalid_argument("BitSet");
            }
        }
        const std::size_t use = rlen < N ? rlen : static_cast<std::size_t>(N);
        for (std::size_t i = 0; i < use; i += 1) {
            if (str[pos + rlen - i - 1] == one) {
                SetBit(static_cast<U32>(i));
            }
        }
    }

    BitSet(const BitSet&) = default;
    BitSet(BitSet&&) = default;
    BitSet& operator=(const BitSet&) = default;
    BitSet& operator=(BitSet&&) = default;

    constexpr U32 size() const {
        return N;
    }

    BitSet& operator&=(const BitSet& rhs) {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            storage[i] = static_cast<Byte>(storage[i] & rhs.storage[i]);
        }
        return *this;
    }

    BitSet& operator|=(const BitSet& rhs) {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            storage[i] = static_cast<Byte>(storage[i] | rhs.storage[i]);
        }
        return *this;
    }

    BitSet& operator^=(const BitSet& rhs) {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            storage[i] = static_cast<Byte>(storage[i] ^ rhs.storage[i]);
        }
        return *this;
    }

    BitSet& operator<<=(U32 n) {
        ShiftLeft(n);
        return *this;
    }

    BitSet& operator>>=(U32 n) {
        ShiftRight(n);
        return *this;
    }

    BitSet operator<<(U32 n) const {
        BitSet out(*this);
        out <<= n;
        return out;
    }

    BitSet operator>>(U32 n) const {
        BitSet out(*this);
        out >>= n;
        return out;
    }

    BitSet operator~() const {
        BitSet out(*this);
        out.flip();
        return out;
    }

    BitSet& set() {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            storage[i] = 0xff;
        }
        ClearTail();
        return *this;
    }

    BitSet& set(U32 pos, bool value = true) {
        Check(pos);
        if (value) {
            SetBit(pos);
        } else {
            ClearBit(pos);
        }
        return *this;
    }

    BitSet& reset() {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            storage[i] = 0;
        }
        return *this;
    }

    BitSet& reset(U32 pos) {
        return set(pos, false);
    }

    BitSet& flip() {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            storage[i] = static_cast<Byte>(~storage[i]);
        }
        ClearTail();
        return *this;
    }

    BitSet& flip(U32 pos) {
        Check(pos);
        storage[pos >> 3] = static_cast<Byte>(storage[pos >> 3] ^ BitMask(pos));
        return *this;
    }

    bool operator[](U32 pos) const {
        return TestBit(pos);
    }

    reference operator[](U32 pos) {
        return reference(*this, pos);
    }

    U32 count() const {
        U32 n = 0;
        for (U32 i = 0; i < StorageBytes; i += 1) {
            Byte v = storage[i];
            while (v != 0) {
                n += v & 1u;
                v = static_cast<Byte>(v >> 1);
            }
        }
        return n;
    }

    bool all() const {
        constexpr U32 full = N >> 3;
        for (U32 i = 0; i < full; i += 1) {
            if (storage[i] != 0xff) {
                return false;
            }
        }
        constexpr U32 rem = N & 7u;
        if (rem == 0) {
            return true;
        }
        const Byte mask = TailMask();
        return (storage[full] & mask) == mask;
    }

    bool any() const {
        for (U32 i = 0; i < StorageBytes; i += 1) {
            if (storage[i] != 0) {
                return true;
            }
        }
        return false;
    }

    bool none() const {
        return !any();
    }

    bool test(U32 pos) const {
        Check(pos);
        return TestBit(pos);
    }

    unsigned long to_ulong() const {
        return ToUnsigned<unsigned long>();
    }

    unsigned long long to_ullong() const {
        return ToUnsigned<unsigned long long>();
    }

    template<class CharT = char, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, Traits, Allocator> to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const {
        std::basic_string<CharT, Traits, Allocator> text(static_cast<std::size_t>(N), zero);
        for (U32 i = 0; i < N; i += 1) {
            if (TestBit(i)) {
                text[static_cast<std::size_t>(N - 1u - i)] = one;
            }
        }
        return text;
    }

    bool operator==(const BitSet& rhs) const {
        return std::memcmp(storage, rhs.storage, StorageBytes) == 0;
    }

    bool operator!=(const BitSet& rhs) const {
        return !(*this == rhs);
    }

    const Byte* data() const {
        return storage;
    }

private:
    static constexpr Byte TailMask() {
        constexpr U32 rem = N & 7u;
        if (rem == 0) {
            return 0xff;
        }
        return static_cast<Byte>((1u << rem) - 1u);
    }

    static constexpr Byte BitMask(U32 index) {
        return static_cast<Byte>(1u << (index & 7u));
    }

    constexpr void ClearTail() {
        constexpr U32 rem = N & 7u;
        if (rem == 0) {
            return;
        }
        storage[StorageBytes - 1] = static_cast<Byte>(storage[StorageBytes - 1] & TailMask());
    }

    void Check(U32 pos) const {
        if (pos >= N) {
            throw std::out_of_range("BitSet");
        }
    }

    void SetBit(U32 index) {
        storage[index >> 3] = static_cast<Byte>(storage[index >> 3] | BitMask(index));
    }

    void ClearBit(U32 index) {
        storage[index >> 3] = static_cast<Byte>(storage[index >> 3] & static_cast<Byte>(~BitMask(index)));
    }

    bool TestBit(U32 index) const {
        return (storage[index >> 3] & BitMask(index)) != 0;
    }

    bool HasBitFrom(U32 bit) const {
        if (bit >= N) {
            return false;
        }
        const U32 byte = bit >> 3;
        const U32 off = bit & 7u;
        const Byte low = static_cast<Byte>((1u << off) - 1u);
        if ((storage[byte] & static_cast<Byte>(~low)) != 0) {
            return true;
        }
        for (U32 i = byte + 1u; i < StorageBytes; i += 1) {
            if (storage[i] != 0) {
                return true;
            }
        }
        return false;
    }

    template<class U>
    U ToUnsigned() const {
        constexpr U32 width = static_cast<U32>(sizeof(U) * 8u);
        if (HasBitFrom(width)) {
            throw std::overflow_error("BitSet");
        }
        U value = 0;
        const U32 n = N < width ? N : width;
        for (U32 i = 0; i < n; i += 1) {
            if (TestBit(i)) {
                value |= static_cast<U>(1) << i;
            }
        }
        return value;
    }

    void ShiftLeft(U32 n) {
        if (n == 0) {
            return;
        }
        if (n >= N) {
            reset();
            return;
        }
        const U32 byteShift = n >> 3;
        const U32 bitShift = n & 7u;
        if (bitShift == 0) {
            std::memmove(storage + byteShift, storage, StorageBytes - byteShift);
            std::memset(storage, 0, byteShift);
        } else {
            const U32 inv = 8u - bitShift;
            for (U32 i = StorageBytes; i-- > 0;) {
                U32 v = 0;
                if (i >= byteShift) {
                    v = static_cast<U32>(storage[i - byteShift]) << bitShift;
                }
                if (i >= byteShift + 1u) {
                    v |= static_cast<U32>(storage[i - byteShift - 1u]) >> inv;
                }
                storage[i] = static_cast<Byte>(v);
            }
        }
        ClearTail();
    }

    void ShiftRight(U32 n) {
        if (n == 0) {
            return;
        }
        if (n >= N) {
            reset();
            return;
        }
        const U32 byteShift = n >> 3;
        const U32 bitShift = n & 7u;
        if (bitShift == 0) {
            std::memmove(storage, storage + byteShift, StorageBytes - byteShift);
            std::memset(storage + (StorageBytes - byteShift), 0, byteShift);
        } else {
            const U32 inv = 8u - bitShift;
            for (U32 i = 0; i < StorageBytes; i += 1) {
                const U32 src = i + byteShift;
                U32 v = 0;
                if (src < StorageBytes) {
                    v = static_cast<U32>(storage[src]) >> bitShift;
                }
                if (src + 1u < StorageBytes) {
                    v |= static_cast<U32>(storage[src + 1u]) << inv;
                }
                storage[i] = static_cast<Byte>(v);
            }
        }
        ClearTail();
    }

    Byte storage[StorageBytes];
};

template<U32 N>
BitSet<N> operator&(BitSet<N> lhs, const BitSet<N>& rhs) {
    lhs &= rhs;
    return lhs;
}

template<U32 N>
BitSet<N> operator|(BitSet<N> lhs, const BitSet<N>& rhs) {
    lhs |= rhs;
    return lhs;
}

template<U32 N>
BitSet<N> operator^(BitSet<N> lhs, const BitSet<N>& rhs) {
    lhs ^= rhs;
    return lhs;
}
