#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>
#include <ostream>
#include <iostream>
#include <cstdlib>
#include "Public/fixed_point/macro.h"

// ============================================================================
// fast_shr_trunc(x, n) - generic helper used by Fixed<> to divide the raw
// intermediate value by 2^FractionBits (i.e. by Scale) after a multiply.
//
// For built-in integer types the compiler already turns "x / (T(1)<<n)"
// into an efficient shift sequence when the divisor is a compile-time
// constant, so the generic template is left as a plain division.
// ============================================================================
template <typename T,
        typename = typename std::enable_if<std::is_integral<T>::value>::type>
FORCE_INLINE constexpr T fast_shr_trunc(T v, int n) {
    return v / (T(1) << n);
}

namespace fixed_point_detail {

template <typename BaseType, typename Store>
struct scaled_divider
{
    FORCE_INLINE static Store divide(Store lhs, Store rhs, int fraction_bits)
    {
        return static_cast<Store>(
            (static_cast<BaseType>(lhs) << fraction_bits) /
            static_cast<BaseType>(rhs));
    }
};

// GCC and Clang otherwise lower signed __int128 division to an out-of-line
// __divti3 call. Fixed<> only keeps the low 64-bit quotient, and its scaled
// numerator is a 128-bit value divided by a 64-bit stored value. x86-64 has
// exactly that operation in hardware, so use divq directly. The uncommon
// overflowing-quotient case first removes the discarded high quotient word,
// leaving the same low 64 bits that the Store cast would have produced.
#if defined(__SIZEOF_INT128__) && defined(__x86_64__) && \
    (defined(__GNUC__) || defined(__clang__)) && !defined(_MSC_VER)
template <>
struct scaled_divider<__int128, std::int64_t>
{
    FORCE_INLINE static std::uint64_t magnitude(std::int64_t value)
    {
        const std::uint64_t bits = static_cast<std::uint64_t>(value);
        const std::uint64_t mask = std::uint64_t(0) - std::uint64_t(value < 0);
        return (bits ^ mask) - mask;
    }

    FORCE_INLINE static std::uint64_t divide_128_by_64_low(
        std::uint64_t high,
        std::uint64_t low,
        std::uint64_t divisor)
    {
        if (high == 0)
            return low / divisor;

        // divq traps when the quotient does not fit in 64 bits. The high
        // quotient word is discarded by Fixed's int64_t storage anyway.
        if (high >= divisor)
            high %= divisor;

        std::uint64_t quotient;
        std::uint64_t remainder;
        __asm__("divq %[divisor]"
                : "=a"(quotient), "=d"(remainder)
                : "a"(low), "d"(high), [divisor] "rm"(divisor)
                : "cc");
        return quotient;
    }

    FORCE_INLINE static std::int64_t divide(
        std::int64_t lhs,
        std::int64_t rhs,
        int fraction_bits)
    {
        if (fraction_bits == 0)
            return lhs / rhs;

        // The optimized construction below covers every practical format
        // whose fractional component fits in its 64-bit Store. Preserve the
        // generic behavior for unusual wider shifts.
        if (fraction_bits < 0 || fraction_bits >= 64) {
            return static_cast<std::int64_t>(
                (static_cast<__int128>(lhs) << fraction_bits) /
                static_cast<__int128>(rhs));
        }

        const bool negative = (lhs < 0) != (rhs < 0);
        const std::uint64_t dividend = magnitude(lhs);
        const std::uint64_t divisor = magnitude(rhs);
        const std::uint64_t high = dividend >> (64 - fraction_bits);
        const std::uint64_t low = dividend << fraction_bits;
        std::uint64_t quotient = divide_128_by_64_low(high, low, divisor);
        if (negative)
            quotient = std::uint64_t(0) - quotient;
        return static_cast<std::int64_t>(quotient);
    }
};
#endif

} // namespace fixed_point_detail
// ============================================================================
// Fixed-point number template
//
//   Fixed<BaseType, Store, FractionBits>
//
//   BaseType     — underlying type for intermediate computation (wider is safer)
//   Store        — storage type (may be smaller than BaseType to save memory)
//   FractionBits — number of fractional bits (0 = plain integer)
//
//   All arithmetic on BaseType and Store must be supported (checked at compile
//   time via static_assert).
// ============================================================================

template <typename BaseType,
          typename Store       = BaseType,
          int FractionBits     = 0>
class Fixed
{
    // ------------------------------------------------------------------------
    // Static sanity checks
    // ------------------------------------------------------------------------
    static_assert(FractionBits < static_cast<int>(sizeof(BaseType) * 8),
                  "FractionBits must be less than the bit-width of BaseType");
    static_assert(FractionBits >= 0,
                  "FractionBits must be non-negative");

    static_assert(sizeof(Store) <= sizeof(BaseType),
                  "Store should not be larger than BaseType");

    // Verify that BaseType and Store support basic arithmetic operations.
    // Each check fails to compile if the operator is missing on that type.
    static_assert(std::is_same<decltype(std::declval<BaseType>() + std::declval<BaseType>()), BaseType>::value
                  && std::is_same<decltype(std::declval<BaseType>() - std::declval<BaseType>()), BaseType>::value
                  && std::is_same<decltype(std::declval<BaseType>() * std::declval<BaseType>()), BaseType>::value
                  && std::is_same<decltype(std::declval<BaseType>() / std::declval<BaseType>()), BaseType>::value
                  && std::is_same<decltype(-std::declval<BaseType>()),                      BaseType>::value,
                  "BaseType must support +, -, *, /, unary -");

    static_assert(std::is_same<decltype(std::declval<Store>() + std::declval<Store>()), Store>::value
                  && std::is_same<decltype(std::declval<Store>() - std::declval<Store>()), Store>::value
                  && std::is_same<decltype(std::declval<Store>() * std::declval<Store>()), Store>::value
                  && std::is_same<decltype(std::declval<Store>() / std::declval<Store>()), Store>::value
                  && std::is_same<decltype(-std::declval<Store>()),                      Store>::value,
                  "Store must support +, -, *, /, unary -");

public:
    // ------------------------------------------------------------------------
    // Types
    // ------------------------------------------------------------------------
    using StorageType    = Store;
    using ComputeType    = BaseType;
    using ThisType       = Fixed<BaseType, Store, FractionBits>;

    static constexpr int kFractionBits = FractionBits;
    static constexpr BaseType Scale=static_cast<BaseType>(uint16_t(1)) << kFractionBits;
    FORCE_INLINE static ThisType Zero(){ ThisType ret; ret.raw_=static_cast<Store>(0); return ret;}
    // ------------------------------------------------------------------------
    // Data
    // ------------------------------------------------------------------------
    Store raw_;

    // ------------------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------------------
    FORCE_INLINE constexpr Fixed() : raw_(0) {}
    FORCE_INLINE constexpr Fixed(const BaseType&val):raw_(val<<kFractionBits){}
    FORCE_INLINE constexpr Fixed(const Fixed&) = default;
    FORCE_INLINE constexpr Fixed& operator=(const Fixed&) = default;

    // Construct from a raw stored value (no scaling applied)
    FORCE_INLINE static  Fixed FromRaw(Store raw) {
        Fixed f;
        f.raw_ = raw;
        return f;
    }

    // Construct from integer or enum (both convert directly to BaseType)
    template <typename T,
              typename = typename std::enable_if<
                  std::is_integral<T>::value || std::is_enum<T>::value>::type>
     FORCE_INLINE constexpr Fixed(T val)
        : raw_(static_cast<Store>(static_cast<BaseType>(val)<<kFractionBits)) {}

    // Compile-time constructor (e.g. Fixed("3.14")).  No error checking —
    // invalid input produces a compile error at  time.
    FORCE_INLINE constexpr Fixed(const char* s)
        : raw_(static_cast<Store>(_parse(s))) {}

    // Runtime constructor from double (non-, for convenience).
    static FORCE_INLINE constexpr ThisType FromDouble(double val)
     {
        ThisType ret;
        ret.raw_=(static_cast<Store>(
            val * static_cast<double>(Scale)
        ));
        return ret;
    }
    static FORCE_INLINE constexpr ThisType FromFloat(float val)
    {
        ThisType ret;
        ret.raw_=(static_cast<Store>(
                   val * static_cast<double>(Scale)
                   ));
        return ret;
    }

    // Runtime string parser with validation.
    // Reports invalid characters and precision overflow, then exits.
    static FORCE_INLINE Fixed FromString(const char* s) {
        // --- sign ---
        bool neg = false;
        if (*s == '-') { neg = true; ++s; }
        else if (*s == '+') { ++s; }

        // --- integer part ---
        BaseType int_part = static_cast<BaseType>(0);
        while (*s >= '0' && *s <= '9') {
            int_part = int_part * static_cast<BaseType>(10)
                     + static_cast<BaseType>(*s - '0');
            ++s;
        }

        // --- fractional part ---
        BaseType frac_part   = static_cast<BaseType>(0);
        int      frac_digits = 0;
        if (*s == '.') {
            ++s;
            while (*s >= '0' && *s <= '9') {
                frac_part = frac_part * static_cast<BaseType>(10)
                          + static_cast<BaseType>(*s - '0');
                ++frac_digits;
                ++s;
            }
        }

        // --- invalid trailing characters ---
        if (*s != '\0') {
            std::cerr << "Fixed::FromString: invalid character '"
                      << *s << "' (0x" << std::hex << (int)(unsigned char)*s
                      << std::dec << ") in \"" << s << "\"" << std::endl;
            std::exit(1);
        }

        // --- assemble ---
        BaseType scale  = static_cast<BaseType>(1) << FractionBits;
        BaseType result = int_part * scale;

        if (frac_digits > 0) {
            BaseType pow10 = static_cast<BaseType>(1);
            for (int i = 0; i < frac_digits; ++i)
                pow10 = pow10 * static_cast<BaseType>(10);

            if (pow10 > scale) {
                std::cerr << "Fixed::FromString: " << frac_digits
                          << " fractional digits exceed "
                          << FractionBits << "-bit precision, value truncated"
                          << std::endl;
            }

            result = result + (frac_part * scale) / pow10;
        }

        return Fixed::FromRaw(static_cast<Store>(neg ? -result : result));
    }

    private:
    // ---- recursive  helpers (C++11-compatible, no loops) ----

    static   BaseType _pow10(int n) {
        return n <= 0 ? static_cast<BaseType>(1)
                      : static_cast<BaseType>(10) * _pow10(n - 1);
    }

    static   BaseType _parse_int(const char* s, BaseType acc) {
        return (*s >= '0' && *s <= '9')
                   ? _parse_int(s + 1, acc * static_cast<BaseType>(10)
                                         + static_cast<BaseType>(*s - '0'))
                   : acc;
    }

    static  int _skip_digits(const char* s) {
        return (*s >= '0' && *s <= '9') ? 1 + _skip_digits(s + 1) : 0;
    }

    static   BaseType _parse_frac(const char* s, int n) {
        return n <= 0 ? static_cast<BaseType>(0)
                      : static_cast<BaseType>(*s - '0') * _pow10(n - 1)
                          + _parse_frac(s + 1, n - 1);
    }

    // ---- main parser ----

    static  FORCE_INLINE BaseType _parse(const char* s) {
        // --- sign ---
        bool neg = false;
        if (*s == '-') { neg = true; ++s; }
        else if (*s == '+') { ++s; }

        // --- integer part ---
        BaseType int_part = _parse_int(s, static_cast<BaseType>(0));
        s += _skip_digits(s);

        // --- fractional part ---
        BaseType frac_part   = static_cast<BaseType>(0);
        int      frac_digits = 0;
        if (*s == '.') {
            ++s;
            frac_digits = _skip_digits(s);
            frac_part   = _parse_frac(s, frac_digits);
            s += frac_digits;
        }

        // --- assemble ---
        BaseType scale  = static_cast<BaseType>(1) << FractionBits;
        BaseType result = int_part * scale;

        if (frac_digits > 0) {
            BaseType pow10 = _pow10(frac_digits);
            result = result + (frac_part * scale) / pow10;
        }

        return neg ? -result : result;
    }

    public:

    // ------------------------------------------------------------------------
    // Conversion operators
    // ------------------------------------------------------------------------
    template <typename Int,
              typename = typename std::enable_if<std::is_integral<Int>::value>::type>
    FORCE_INLINE constexpr  operator Int() const {
        // raw_ is already stored at Store's width; dividing directly in
        // Store avoids promoting to the (possibly much slower, e.g. Int128)
        // BaseType just to perform a division that Store can do natively.
        return static_cast<Int>(raw_ / static_cast<Store>(Scale));
    }
    FORCE_INLINE constexpr  explicit operator float() const {
        // Go through double to avoid missing Int128->float conversion
        return static_cast<float>(static_cast<double>(raw_) / static_cast<double>(Scale));
    }

    FORCE_INLINE constexpr operator double() const {
        // Direct conversion from raw fixed-point value to double.
        // Avoids going through BaseType (Int128) which has broken negative conversion.
        return static_cast<double>(raw_) / static_cast<double>(Scale);
    }

    // ------------------------------------------------------------------------
    // Raw access
    // ------------------------------------------------------------------------
    FORCE_INLINE   Store raw() const { return raw_; }

    // ------------------------------------------------------------------------
    // Unary operators
    // ------------------------------------------------------------------------
    FORCE_INLINE   Fixed operator+() const { return *this; }
    FORCE_INLINE   Fixed operator-() const { return Fixed::FromRaw(static_cast<Store>(-raw_)); }

    // ------------------------------------------------------------------------
    // Comparison operators
    // ------------------------------------------------------------------------
    FORCE_INLINE   bool operator==(const Fixed& rhs) const { return raw_ == rhs.raw_; }
    FORCE_INLINE   bool operator!=(const Fixed& rhs) const { return !(raw_ == rhs.raw_); }
    FORCE_INLINE   bool operator< (const Fixed& rhs) const { return raw_ <  rhs.raw_; }
    FORCE_INLINE   bool operator> (const Fixed& rhs) const { return rhs.raw_<raw_; }
    FORCE_INLINE   bool operator<=(const Fixed& rhs) const { return !(raw_ > rhs.raw_); }
    FORCE_INLINE   bool operator>=(const Fixed& rhs) const { return !(raw_ < rhs.raw_); }

    // ------------------------------------------------------------------------
    // Arithmetic: Fixed OP Fixed
    // ------------------------------------------------------------------------
    FORCE_INLINE   Fixed operator+(const Fixed& rhs) const {
        return Fixed::FromRaw(static_cast<Store>(raw_ + rhs.raw_));
    }

    FORCE_INLINE  Fixed operator-(const Fixed& rhs) const {
        return Fixed::FromRaw(static_cast<Store>(raw_ - rhs.raw_));
    }

    FORCE_INLINE   Fixed operator*(const Fixed& rhs) const {
        return Fixed::FromRaw(static_cast<Store>(
            fast_shr_trunc(static_cast<BaseType>(raw_) * static_cast<BaseType>(rhs.raw_), FractionBits)));
    }

    FORCE_INLINE constexpr Fixed operator/(const Fixed& rhs) const {
        return Fixed::FromRaw(
            fixed_point_detail::scaled_divider<BaseType, Store>::divide(
                raw_, rhs.raw_, FractionBits));
    }

    FORCE_INLINE  constexpr Fixed operator%(const Fixed& rhs) const {
        return Fixed::FromRaw(static_cast<Store>((raw_%rhs.raw_)));
    }

    // ------------------------------------------------------------------------
    // Arithmetic: Fixed OP integer
    // ------------------------------------------------------------------------

    template <typename Int>
    FORCE_INLINE   auto operator+(Int rhs) const ->
        typename std::enable_if<std::is_integral<Int>::value, Fixed>::type
    {
        return *this + Fixed(static_cast<BaseType>(rhs));
    }

    template <typename Int>
    FORCE_INLINE   auto operator-(Int rhs) const ->
        typename std::enable_if<std::is_integral<Int>::value, Fixed>::type
    {
        return *this - Fixed(static_cast<BaseType>(rhs));
    }

    template <typename Int>
    FORCE_INLINE   auto operator*(Int rhs) const ->
        typename std::enable_if<std::is_integral<Int>::value, Fixed>::type
    {
        return Fixed::FromRaw(static_cast<Store>(
            static_cast<BaseType>(raw_) *
            static_cast<BaseType>(rhs)
        ));
    }

    template <typename Int>
    FORCE_INLINE   auto operator/(Int rhs) const ->
        typename std::enable_if<std::is_integral<Int>::value, Fixed>::type
    {
        return Fixed::FromRaw(static_cast<Store>(
            static_cast<BaseType>(raw_) / static_cast<BaseType>(rhs)));
    }

    // ------------------------------------------------------------------------
    // Compound-assignment: Fixed OP Fixed
    // ------------------------------------------------------------------------
    FORCE_INLINE   Fixed& operator+=(const Fixed& rhs) {
        raw_ = static_cast<Store>(raw_ + rhs.raw_);
        return *this;
    }

    FORCE_INLINE  Fixed& operator-=(const Fixed& rhs) {
        raw_ = static_cast<Store>(raw_ - rhs.raw_);
        return *this;
    }

    FORCE_INLINE   Fixed& operator*=(const Fixed& rhs) {
        raw_ = static_cast<Store>(
            fast_shr_trunc(static_cast<BaseType>(raw_) * static_cast<BaseType>(rhs.raw_), FractionBits));
        return *this;
    }

    FORCE_INLINE   Fixed& operator/=(const Fixed& rhs) {
        raw_ = fixed_point_detail::scaled_divider<BaseType, Store>::divide(
            raw_, rhs.raw_, FractionBits);
        return *this;
    }

    FORCE_INLINE   Fixed& operator%=(const Fixed& rhs) {
        raw_ = static_cast<Store>(raw_ % rhs.raw_);
        return *this;
    }

    // ------------------------------------------------------------------------
    // Increment / Decrement
    // ------------------------------------------------------------------------
    FORCE_INLINE   Fixed& operator++() { *this += Fixed(1); return *this; }
    FORCE_INLINE   Fixed  operator++(int) { Fixed tmp = *this; ++*this; return tmp; }
    FORCE_INLINE   Fixed& operator--() { *this -= Fixed(1); return *this; }
    FORCE_INLINE   Fixed  operator--(int) { Fixed tmp = *this; --*this; return tmp; }

    // ------------------------------------------------------------------------
    // Absolute value
    // ------------------------------------------------------------------------
    FORCE_INLINE   Fixed abs() const {
        return Fixed::FromRaw(raw_ < 0 ? static_cast<Store>(-raw_) : raw_);
    }

    // ------------------------------------------------------------------------
    // Round to nearest integer (returning a Fixed still in the same format)
    // ------------------------------------------------------------------------
    FORCE_INLINE Fixed round() const {
        Store v = raw_;
        constexpr Store s = static_cast<Store>(Scale);   // 编译期已知的2的幂
        constexpr Store mask = s - 1;
        if (v >= 0) {
            return Fixed::FromRaw(static_cast<Store>((v + s / 2) & ~mask));
        } else {
            Store nv = -v;
            return Fixed::FromRaw(static_cast<Store>(-((nv + s / 2) & ~mask)));
        }
    }

    FORCE_INLINE Fixed floor() const {
        constexpr Store s = static_cast<Store>(Scale);
        constexpr Store mask = s - 1;
        return Fixed::FromRaw(static_cast<Store>(raw_ & ~mask));
    }

    FORCE_INLINE Fixed ceil() const {
        constexpr Store s = static_cast<Store>(Scale);
        constexpr Store mask = s - 1;
        return Fixed::FromRaw(static_cast<Store>((raw_ + mask) & ~mask));
    }

    FORCE_INLINE Fixed trunc() const {
        constexpr Store s = static_cast<Store>(Scale);
        constexpr Store mask = s - 1;
        // v >> 63 是算术右移：v为负时全1，v非负时全0，用它当"是否要补偏移量"的开关
        Store sign_mask = raw_ >> (sizeof(Store) * 8 - 1);
        Store adj = sign_mask & mask;
        return Fixed::FromRaw(static_cast<Store>((raw_ + adj) & ~mask));
    }

    // ------------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------------
    FORCE_INLINE  static  Fixed min() {
        return Fixed::FromRaw(std::numeric_limits<Store>::min());
    }
    FORCE_INLINE  static  Fixed max() {
        return Fixed::FromRaw(std::numeric_limits<Store>::max());
    }
    FORCE_INLINE   static  Fixed epsilon() {
        return Fixed::FromRaw(static_cast<Store>(1));
    }
    FORCE_INLINE  static  Fixed pi() {
        return Fixed::FromDouble(3.14159265358979323846);
    }
    FORCE_INLINE  static  Fixed e() {
        return Fixed::FromDouble(2.71828182845904523536);
    }

};

// ============================================================================
// Symmetric operators:  integer OP Fixed
// ============================================================================

    template <typename BaseType, typename Store, int FBits, typename Int>
    FORCE_INLINE  auto operator+(Int lhs, const Fixed<BaseType, Store, FBits>& rhs) ->
    typename std::enable_if<std::is_integral<Int>::value,
                            Fixed<BaseType, Store, FBits>>::type
    {
    return rhs + lhs;
    }

    template <typename BaseType, typename Store, int FBits, typename Int>
    FORCE_INLINE auto operator-(Int lhs, const Fixed<BaseType, Store, FBits>& rhs) ->
    typename std::enable_if<std::is_integral<Int>::value,
                            Fixed<BaseType, Store, FBits>>::type
    {
    return Fixed<BaseType, Store, FBits>(static_cast<BaseType>(lhs)) - rhs;
    }

    template <typename BaseType, typename Store, int FBits, typename Int>
    FORCE_INLINE  auto operator*(Int lhs, const Fixed<BaseType, Store, FBits>& rhs) ->
    typename std::enable_if<std::is_integral<Int>::value,
                            Fixed<BaseType, Store, FBits>>::type
    {
    return rhs * lhs;
    }

    template <typename BaseType, typename Store, int FBits, typename Int>
    FORCE_INLINE auto operator/(Int lhs, const Fixed<BaseType, Store, FBits>& rhs) ->
    typename std::enable_if<std::is_integral<Int>::value,
                            Fixed<BaseType, Store, FBits>>::type
    {
    return Fixed<BaseType, Store, FBits>(static_cast<BaseType>(lhs)) / rhs;
    }

    // ============================================================================
    // Stream output
    // ============================================================================

    template <typename BaseType, typename Store, int FractionBits>
    FORCE_INLINE std::ostream& operator<<(std::ostream& os, const Fixed<BaseType, Store, FractionBits>& v) {
    return os << static_cast<double>(v);
    }

    // ============================================================================
    // Convenience aliases
    // ============================================================================

    // Q-format nomenclature: Qm.n where m = integer bits, n = FractionBits
    // Storage = BaseType

    template <int FractionBits>
    using FixedQ = Fixed<int32_t, int32_t, FractionBits>;

    template <int FractionBits>
    using FixedQL = Fixed<int64_t, int64_t, FractionBits>;

    // Common fixed-point formats
    using Fixed16_16  = Fixed<int32_t, int32_t, 16>;   // Q16.16
    using Fixed8_24   = Fixed<int32_t, int32_t, 24>;   // Q8.24
    using Fixed24_8   = Fixed<int32_t, int32_t, 8>;    // Q24.8
    using Fixed32_32  = Fixed<int64_t, int64_t, 32>;   // Q32.32
    using Fixed16      = Fixed<int32_t, int32_t, 16>;  // legacy alias
