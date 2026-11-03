#pragma once

#include "Public/fixed_point/include.h"
#include "Public/type/type.h"
#include "Public/util/util.h"

#include <cmath>
#include <limits>
#include <type_traits>

// Two-component signed integer vector. The type deliberately stays a plain
// eight-byte value so copies and by-value parameters fit in registers on the
// project's target platforms.
class IVec2 final {
public:
    using ValueType = I32;

    I32 x = 0;
    I32 y = 0;

    FORCE_INLINE constexpr IVec2() noexcept = default;
    FORCE_INLINE constexpr IVec2(I32 xValue, I32 yValue) noexcept
        : x(xValue), y(yValue) {}
    FORCE_INLINE explicit constexpr IVec2(I32 value) noexcept
        : x(value), y(value) {}
    FORCE_INLINE constexpr IVec2(const IVec2&) noexcept = default;
    FORCE_INLINE constexpr IVec2(IVec2&&) noexcept = default;
    FORCE_INLINE constexpr IVec2& operator=(const IVec2&) noexcept = default;
    FORCE_INLINE constexpr IVec2& operator=(IVec2&&) noexcept = default;

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 Zero() noexcept {
        return {};
    }

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 One() noexcept {
        return {1, 1};
    }

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 UnitX() noexcept {
        return {1, 0};
    }

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 UnitY() noexcept {
        return {0, 1};
    }

    // Unchecked for speed. The caller must pass 0 or 1.
    [[nodiscard]] FORCE_INLINE constexpr I32& operator[](U32 index) noexcept {
        return index == 0 ? x : y;
    }

    [[nodiscard]] FORCE_INLINE constexpr const I32& operator[](U32 index) const noexcept {
        return index == 0 ? x : y;
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator+() const noexcept {
        return *this;
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator-() const noexcept {
        return {-x, -y};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator+(IVec2 rhs) const noexcept {
        return {x + rhs.x, y + rhs.y};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator-(IVec2 rhs) const noexcept {
        return {x - rhs.x, y - rhs.y};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator*(IVec2 rhs) const noexcept {
        return {x * rhs.x, y * rhs.y};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator/(IVec2 rhs) const noexcept {
        return {x / rhs.x, y / rhs.y};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator%(IVec2 rhs) const noexcept {
        return {x % rhs.x, y % rhs.y};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator*(I32 scalar) const noexcept {
        return {x * scalar, y * scalar};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator/(I32 scalar) const noexcept {
        return {x / scalar, y / scalar};
    }

    [[nodiscard]] FORCE_INLINE constexpr IVec2 operator%(I32 scalar) const noexcept {
        return {x % scalar, y % scalar};
    }

    FORCE_INLINE constexpr IVec2& operator+=(IVec2 rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator-=(IVec2 rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator*=(IVec2 rhs) noexcept {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator/=(IVec2 rhs) noexcept {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator%=(IVec2 rhs) noexcept {
        x %= rhs.x;
        y %= rhs.y;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator*=(I32 scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator/=(I32 scalar) noexcept {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    FORCE_INLINE constexpr IVec2& operator%=(I32 scalar) noexcept {
        x %= scalar;
        y %= scalar;
        return *this;
    }

    [[nodiscard]] FORCE_INLINE constexpr bool operator==(IVec2 rhs) const noexcept {
        return x == rhs.x && y == rhs.y;
    }

    [[nodiscard]] FORCE_INLINE constexpr bool operator!=(IVec2 rhs) const noexcept {
        return !(*this == rhs);
    }

    [[nodiscard]] FORCE_INLINE constexpr bool IsZero() const noexcept {
        return x == 0 && y == 0;
    }

    [[nodiscard]] FORCE_INLINE constexpr I64 Dot(IVec2 rhs) const noexcept {
        return static_cast<I64>(x) * static_cast<I64>(rhs.x) +
               static_cast<I64>(y) * static_cast<I64>(rhs.y);
    }

    [[nodiscard]] FORCE_INLINE constexpr U64 LengthSquared() const noexcept {
        const I64 wideX = static_cast<I64>(x);
        const I64 wideY = static_cast<I64>(y);
        return static_cast<U64>(wideX * wideX) +
               static_cast<U64>(wideY * wideY);
    }

    [[nodiscard]] FORCE_INLINE Float64 Length() const noexcept {
        return std::sqrt(static_cast<Float64>(LengthSquared()));
    }

    [[nodiscard]] FORCE_INLINE constexpr U64 ManhattanLength() const noexcept {
        return UnsignedMagnitude(x) + UnsignedMagnitude(y);
    }

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 Min(IVec2 lhs,
                                                           IVec2 rhs) noexcept {
        return {lhs.x < rhs.x ? lhs.x : rhs.x,
                lhs.y < rhs.y ? lhs.y : rhs.y};
    }

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 Max(IVec2 lhs,
                                                           IVec2 rhs) noexcept {
        return {lhs.x > rhs.x ? lhs.x : rhs.x,
                lhs.y > rhs.y ? lhs.y : rhs.y};
    }

    [[nodiscard]] static FORCE_INLINE constexpr IVec2 Clamp(
        IVec2 value,
        IVec2 minimum,
        IVec2 maximum) noexcept {
        return Min(Max(value, minimum), maximum);
    }

private:
    [[nodiscard]] static FORCE_INLINE constexpr U64 UnsignedMagnitude(
        I32 value) noexcept {
        return value < 0
            ? static_cast<U64>(-static_cast<I64>(value))
            : static_cast<U64>(value);
    }
};

[[nodiscard]] FORCE_INLINE constexpr IVec2 operator*(I32 scalar,
                                                      IVec2 value) noexcept {
    return value * scalar;
}

// Signed Q39.24 vector. Each component has 24 fractional bits and a range of
// [-549755813888, 549755813887.999999940395355224609375].
class Vec2 final {
public:
    using ValueType = Fixed<I64, I64, 24>;
    static constexpr U32 FractionBits = 24;
    static constexpr I64 Scale = I64{1} << FractionBits;

    ValueType x{};
    ValueType y{};

    FORCE_INLINE constexpr Vec2() noexcept = default;
    FORCE_INLINE constexpr Vec2(ValueType xValue, ValueType yValue) noexcept
        : x(xValue), y(yValue) {}
    FORCE_INLINE explicit constexpr Vec2(ValueType value) noexcept
        : x(value), y(value) {}
    // These overloads avoid Fixed's signed-negative left shift when ordinary
    // integer coordinates such as Vec2{-1, 2} are used. Constraining them to
    // integral inputs prevents accidental truncation of floating-point values.
    template<typename X, typename Y,
             typename = typename std::enable_if<
                 std::is_integral<X>::value &&
                 std::is_integral<Y>::value>::type>
    FORCE_INLINE Vec2(X xValue, Y yValue) noexcept
        : x(ValueType::FromRaw(IntegerRaw(xValue))),
          y(ValueType::FromRaw(IntegerRaw(yValue))) {}

    template<typename Integer,
             typename = typename std::enable_if<
                 std::is_integral<Integer>::value>::type>
    FORCE_INLINE explicit Vec2(Integer value) noexcept
        : x(ValueType::FromRaw(IntegerRaw(value))),
          y(ValueType::FromRaw(IntegerRaw(value))) {}

    template<typename X, typename Y,
             typename std::enable_if<
                 std::is_floating_point<X>::value ||
                 std::is_floating_point<Y>::value, int>::type = 0>
    Vec2(X, Y) = delete;

    template<typename Floating,
             typename std::enable_if<
                 std::is_floating_point<Floating>::value, int>::type = 0>
    explicit Vec2(Floating) = delete;
    FORCE_INLINE constexpr Vec2(const Vec2&) noexcept = default;
    FORCE_INLINE constexpr Vec2(Vec2&&) noexcept = default;
    FORCE_INLINE constexpr Vec2& operator=(const Vec2&) noexcept = default;
    FORCE_INLINE constexpr Vec2& operator=(Vec2&&) noexcept = default;

    [[nodiscard]] static FORCE_INLINE constexpr Vec2 Zero() noexcept {
        return {};
    }

    [[nodiscard]] static FORCE_INLINE constexpr Vec2 One() noexcept {
        return {ValueType(1), ValueType(1)};
    }

    [[nodiscard]] static FORCE_INLINE constexpr Vec2 UnitX() noexcept {
        return {ValueType(1), ValueType(0)};
    }

    [[nodiscard]] static FORCE_INLINE constexpr Vec2 UnitY() noexcept {
        return {ValueType(0), ValueType(1)};
    }

    [[nodiscard]] static FORCE_INLINE Vec2 FromRaw(I64 xRaw,
                                                   I64 yRaw) noexcept {
        return {ValueType::FromRaw(xRaw), ValueType::FromRaw(yRaw)};
    }

    // Inputs must be inside ValueType's signed representable range.
    [[nodiscard]] static FORCE_INLINE Vec2 FromFloat(Float32 xValue,
                                                     Float32 yValue) noexcept {
        return {ValueType::FromFloat(xValue), ValueType::FromFloat(yValue)};
    }

    [[nodiscard]] static FORCE_INLINE Vec2 FromDouble(Float64 xValue,
                                                      Float64 yValue) noexcept {
        return {ValueType::FromDouble(xValue), ValueType::FromDouble(yValue)};
    }

    // Unchecked for speed. The caller must pass 0 or 1.
    [[nodiscard]] FORCE_INLINE ValueType& operator[](U32 index) noexcept {
        return index == 0 ? x : y;
    }

    [[nodiscard]] FORCE_INLINE const ValueType& operator[](U32 index) const noexcept {
        return index == 0 ? x : y;
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator+() const noexcept {
        return *this;
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator-() const noexcept {
        return {-x, -y};
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator+(const Vec2& rhs) const noexcept {
        return {x + rhs.x, y + rhs.y};
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator-(const Vec2& rhs) const noexcept {
        return {x - rhs.x, y - rhs.y};
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator*(const Vec2& rhs) const noexcept {
        return {Multiply(x, rhs.x), Multiply(y, rhs.y)};
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator/(const Vec2& rhs) const noexcept {
        return {Divide(x, rhs.x), Divide(y, rhs.y)};
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator*(ValueType scalar) const noexcept {
        return {Multiply(x, scalar), Multiply(y, scalar)};
    }

    [[nodiscard]] FORCE_INLINE Vec2 operator/(ValueType scalar) const noexcept {
        return {Divide(x, scalar), Divide(y, scalar)};
    }

    template<typename Integer,
             typename std::enable_if<
                 std::is_integral<Integer>::value, int>::type = 0>
    [[nodiscard]] FORCE_INLINE Vec2 operator*(Integer scalar) const noexcept {
        return {MultiplyInteger(x, scalar), MultiplyInteger(y, scalar)};
    }

    template<typename Integer,
             typename std::enable_if<
                 std::is_integral<Integer>::value, int>::type = 0>
    [[nodiscard]] FORCE_INLINE Vec2 operator/(Integer scalar) const noexcept {
        return {DivideInteger(x, scalar), DivideInteger(y, scalar)};
    }

    template<typename Floating,
             typename std::enable_if<
                 std::is_floating_point<Floating>::value, int>::type = 0>
    Vec2 operator*(Floating) const = delete;

    template<typename Floating,
             typename std::enable_if<
                 std::is_floating_point<Floating>::value, int>::type = 0>
    Vec2 operator/(Floating) const = delete;

    FORCE_INLINE Vec2& operator+=(const Vec2& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    FORCE_INLINE Vec2& operator-=(const Vec2& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    FORCE_INLINE Vec2& operator*=(const Vec2& rhs) noexcept {
        x = Multiply(x, rhs.x);
        y = Multiply(y, rhs.y);
        return *this;
    }

    FORCE_INLINE Vec2& operator/=(const Vec2& rhs) noexcept {
        x = Divide(x, rhs.x);
        y = Divide(y, rhs.y);
        return *this;
    }

    FORCE_INLINE Vec2& operator*=(ValueType scalar) noexcept {
        x = Multiply(x, scalar);
        y = Multiply(y, scalar);
        return *this;
    }

    FORCE_INLINE Vec2& operator/=(ValueType scalar) noexcept {
        x = Divide(x, scalar);
        y = Divide(y, scalar);
        return *this;
    }

    template<typename Integer,
             typename std::enable_if<
                 std::is_integral<Integer>::value, int>::type = 0>
    FORCE_INLINE Vec2& operator*=(Integer scalar) noexcept {
        x = MultiplyInteger(x, scalar);
        y = MultiplyInteger(y, scalar);
        return *this;
    }

    template<typename Integer,
             typename std::enable_if<
                 std::is_integral<Integer>::value, int>::type = 0>
    FORCE_INLINE Vec2& operator/=(Integer scalar) noexcept {
        x = DivideInteger(x, scalar);
        y = DivideInteger(y, scalar);
        return *this;
    }

    template<typename Floating,
             typename std::enable_if<
                 std::is_floating_point<Floating>::value, int>::type = 0>
    Vec2& operator*=(Floating) = delete;

    template<typename Floating,
             typename std::enable_if<
                 std::is_floating_point<Floating>::value, int>::type = 0>
    Vec2& operator/=(Floating) = delete;

    [[nodiscard]] FORCE_INLINE bool operator==(const Vec2& rhs) const noexcept {
        return x == rhs.x && y == rhs.y;
    }

    [[nodiscard]] FORCE_INLINE bool operator!=(const Vec2& rhs) const noexcept {
        return !(*this == rhs);
    }

    [[nodiscard]] FORCE_INLINE bool IsZero() const noexcept {
        return x.raw() == 0 && y.raw() == 0;
    }

    [[nodiscard]] FORCE_INLINE ValueType Dot(const Vec2& rhs) const noexcept {
        return Multiply(x, rhs.x) + Multiply(y, rhs.y);
    }

    [[nodiscard]] FORCE_INLINE ValueType LengthSquared() const noexcept {
        return Multiply(x, x) + Multiply(y, y);
    }

    [[nodiscard]] FORCE_INLINE ValueType Length() const noexcept {
        return HypotRaw(UnsignedMagnitude(x.raw()),
                        UnsignedMagnitude(y.raw()));
    }

    [[nodiscard]] FORCE_INLINE Vec2 Normalized() const noexcept {
        const I64 rawX = x.raw();
        const I64 rawY = y.raw();
        const U64 magnitudeX = UnsignedMagnitude(rawX);
        const U64 magnitudeY = UnsignedMagnitude(rawY);
        const bool xIsMajor = magnitudeX >= magnitudeY;
        const U64 major = xIsMajor ? magnitudeX : magnitudeY;
        if (major == 0) {
            return Zero();
        }

        // Scale by the major component so normalization keeps its precision
        // for both tiny vectors and large world coordinates.
        constexpr U64 scale = U64{1} << FractionBits;
        const U64 minor = xIsMajor ? magnitudeY : magnitudeX;
        const U64 ratio = ScaledRatio(minor, major);
        const U64 denominator = IntegerSquareRoot(scale * scale + ratio * ratio);
        const U64 majorNormalized = (scale * scale) / denominator;
        const U64 minorNormalized =
            (ratio * majorNormalized) >> FractionBits;

        const I64 normalizedX = ApplySign(
            xIsMajor ? majorNormalized : minorNormalized,
            rawX < 0);
        const I64 normalizedY = ApplySign(
            xIsMajor ? minorNormalized : majorNormalized,
            rawY < 0);
        return FromRaw(normalizedX, normalizedY);
    }

    FORCE_INLINE bool Normalize() noexcept {
        if (IsZero()) {
            return false;
        }
        *this = Normalized();
        return true;
    }

    [[nodiscard]] FORCE_INLINE ValueType DistanceSquared(
        const Vec2& rhs) const noexcept {
        const ValueType deltaX = x - rhs.x;
        const ValueType deltaY = y - rhs.y;
        return Multiply(deltaX, deltaX) + Multiply(deltaY, deltaY);
    }

    [[nodiscard]] FORCE_INLINE ValueType Distance(
        const Vec2& rhs) const noexcept {
        return HypotRaw(DifferenceMagnitude(x.raw(), rhs.x.raw()),
                        DifferenceMagnitude(y.raw(), rhs.y.raw()));
    }

    [[nodiscard]] static FORCE_INLINE Vec2 Min(const Vec2& lhs,
                                               const Vec2& rhs) noexcept {
        return {lhs.x < rhs.x ? lhs.x : rhs.x,
                lhs.y < rhs.y ? lhs.y : rhs.y};
    }

    [[nodiscard]] static FORCE_INLINE Vec2 Max(const Vec2& lhs,
                                               const Vec2& rhs) noexcept {
        return {lhs.x > rhs.x ? lhs.x : rhs.x,
                lhs.y > rhs.y ? lhs.y : rhs.y};
    }

    [[nodiscard]] static FORCE_INLINE Vec2 Clamp(const Vec2& value,
                                                 const Vec2& minimum,
                                                 const Vec2& maximum) noexcept {
        return {
            value.x < minimum.x
                ? minimum.x
                : (value.x > maximum.x ? maximum.x : value.x),
            value.y < minimum.y
                ? minimum.y
                : (value.y > maximum.y ? maximum.y : value.y)
        };
    }

private:
    template<typename Integer>
    [[nodiscard]] static FORCE_INLINE I64 IntegerRaw(Integer value) noexcept {
        constexpr I64 maximumInteger =
            std::numeric_limits<I64>::max() / Scale;
        constexpr I64 minimumInteger =
            std::numeric_limits<I64>::min() / Scale;
        if constexpr (std::is_signed<Integer>::value) {
            const I64 converted = static_cast<I64>(value);
            if (converted > maximumInteger) {
                return std::numeric_limits<I64>::max();
            }
            if (converted < minimumInteger) {
                return std::numeric_limits<I64>::min();
            }
            return converted * Scale;
        } else {
            const U64 converted = static_cast<U64>(value);
            if (converted > static_cast<U64>(maximumInteger)) {
                return std::numeric_limits<I64>::max();
            }
            return static_cast<I64>(converted) * Scale;
        }
    }

    [[nodiscard]] static FORCE_INLINE U64 UnsignedMagnitude(I64 value) noexcept {
        const U64 bits = static_cast<U64>(value);
        return value < 0 ? U64{0} - bits : bits;
    }

    template<typename Integer>
    [[nodiscard]] static FORCE_INLINE U64 IntegerMagnitude(
        Integer value) noexcept {
        if constexpr (std::is_signed<Integer>::value) {
            return UnsignedMagnitude(static_cast<I64>(value));
        } else {
            return static_cast<U64>(value);
        }
    }

    template<typename Integer>
    [[nodiscard]] static FORCE_INLINE bool IntegerIsNegative(
        Integer value) noexcept {
        if constexpr (std::is_signed<Integer>::value) {
            return value < 0;
        } else {
            return false;
        }
    }

    [[nodiscard]] static FORCE_INLINE ValueType Multiply(
        ValueType lhs,
        ValueType rhs) noexcept {
        const bool negative = (lhs.raw() < 0) != (rhs.raw() < 0);
        const U64 product = UnsignedMagnitude(lhs.raw()) *
                            UnsignedMagnitude(rhs.raw());
        return ValueType::FromRaw(
            ApplySign(product >> FractionBits, negative));
    }

    template<typename Integer>
    [[nodiscard]] static FORCE_INLINE ValueType MultiplyInteger(
        ValueType lhs,
        Integer rhs) noexcept {
        const bool negative = (lhs.raw() < 0) != IntegerIsNegative(rhs);
        const U64 product = UnsignedMagnitude(lhs.raw()) *
                            IntegerMagnitude(rhs);
        return ValueType::FromRaw(ApplySign(product, negative));
    }

    [[nodiscard]] static FORCE_INLINE ValueType Divide(
        ValueType lhs,
        ValueType rhs) noexcept {
        const bool negative = (lhs.raw() < 0) != (rhs.raw() < 0);
        const U64 numerator = UnsignedMagnitude(lhs.raw());
        const U64 denominator = UnsignedMagnitude(rhs.raw());
        if (numerator <=
            (std::numeric_limits<U64>::max() >> FractionBits)) {
            const U64 magnitude =
                (numerator << FractionBits) / denominator;
            return ValueType::FromRaw(ApplySign(magnitude, negative));
        }

        const U64 integerPart = numerator / denominator;
        const U64 remainder = numerator % denominator;
        constexpr U64 maximumMagnitude = U64{1} << 63;

        if (integerPart >= (maximumMagnitude >> FractionBits)) {
            return ValueType::FromRaw(
                ApplySign(maximumMagnitude, negative));
        }
        const U64 magnitude =
            (integerPart << FractionBits) +
            ScaledRatio(remainder, denominator);
        return ValueType::FromRaw(ApplySign(magnitude, negative));
    }

    template<typename Integer>
    [[nodiscard]] static FORCE_INLINE ValueType DivideInteger(
        ValueType lhs,
        Integer rhs) noexcept {
        const bool negative = (lhs.raw() < 0) != IntegerIsNegative(rhs);
        const U64 magnitude = UnsignedMagnitude(lhs.raw()) /
                              IntegerMagnitude(rhs);
        return ValueType::FromRaw(ApplySign(magnitude, negative));
    }

    [[nodiscard]] static FORCE_INLINE U64 DifferenceMagnitude(
        I64 lhs,
        I64 rhs) noexcept {
        const bool lhsNegative = lhs < 0;
        const bool rhsNegative = rhs < 0;
        const U64 lhsMagnitude = UnsignedMagnitude(lhs);
        const U64 rhsMagnitude = UnsignedMagnitude(rhs);
        if (lhsNegative != rhsNegative) {
            return lhsMagnitude + rhsMagnitude;
        }
        return lhsMagnitude >= rhsMagnitude
            ? lhsMagnitude - rhsMagnitude
            : rhsMagnitude - lhsMagnitude;
    }

    [[nodiscard]] static FORCE_INLINE U64 ScaledRatio(U64 numerator,
                                                       U64 denominator) noexcept {
        constexpr U64 scale = U64{1} << FractionBits;
        if (numerator == denominator) {
            return scale;
        }
        if (numerator <= (std::numeric_limits<U64>::max() >> FractionBits)) {
            return (numerator << FractionBits) / denominator;
        }
        return ScaledRatioSlow(numerator, denominator);
    }

    // Kept out of hot callers: this only runs for raw magnitudes above 2^40.
    [[nodiscard]] static NO_INLINE U64 ScaledRatioSlow(
        U64 numerator,
        U64 denominator) noexcept {
        U64 remainder = numerator;
        U64 result = 0;
        for (U32 bit = 0; bit < FractionBits; ++bit) {
            result <<= 1;
            if (remainder >= denominator - remainder) {
                remainder -= denominator - remainder;
                result |= 1;
            } else {
                remainder += remainder;
            }
        }
        return result;
    }

    [[nodiscard]] static FORCE_INLINE U32 IntegerSquareRoot(U64 value) noexcept {
        constexpr U64 maxRaw = std::numeric_limits<U32>::max();
        U64 result = static_cast<U64>(
            std::sqrt(static_cast<Float64>(value)));
        if (result > maxRaw) {
            result = maxRaw;
        }

        // Floating point supplies a fast hardware estimate. Integer checks
        // make the returned floor-sqrt exact and platform-independent.
        if (result * result > value) {
            --result;
        } else if (result < maxRaw) {
            const U64 next = result + 1;
            if (next * next <= value) {
                result = next;
            }
        }
        return static_cast<U32>(result);
    }

    [[nodiscard]] static FORCE_INLINE ValueType HypotRaw(U64 lhs,
                                                         U64 rhs) noexcept {
        constexpr U64 maxSquareOperand = std::numeric_limits<U32>::max();
        if (lhs <= maxSquareOperand && rhs <= maxSquareOperand) {
            const U64 lhsSquared = lhs * lhs;
            const U64 rhsSquared = rhs * rhs;
            if (lhsSquared <= std::numeric_limits<U64>::max() - rhsSquared) {
                return ValueType::FromRaw(static_cast<I64>(
                    IntegerSquareRoot(lhsSquared + rhsSquared)));
            }
        }

        return HypotFallback(lhs, rhs);
    }

    struct WideUnsigned {
        U64 high;
        U64 low;
    };

    [[nodiscard]] static FORCE_INLINE WideUnsigned SquareWide(
        U64 value) noexcept {
        constexpr U64 mask = 0xffffffffULL;
        const U64 lowPart = value & mask;
        const U64 highPart = value >> 32;
        const U64 lowLow = lowPart * lowPart;
        const U64 lowHigh = lowPart * highPart;
        const U64 highLow = highPart * lowPart;
        const U64 highHigh = highPart * highPart;
        const U64 middle = (lowLow >> 32) +
                           (lowHigh & mask) +
                           (highLow & mask);
        return {
            highHigh + (lowHigh >> 32) + (highLow >> 32) + (middle >> 32),
            (middle << 32) | (lowLow & mask)
        };
    }

    [[nodiscard]] static FORCE_INLINE WideUnsigned AddWide(
        WideUnsigned lhs,
        WideUnsigned rhs) noexcept {
        const U64 low = lhs.low + rhs.low;
        return {lhs.high + rhs.high + static_cast<U64>(low < lhs.low), low};
    }

    [[nodiscard]] static FORCE_INLINE bool GreaterWide(
        WideUnsigned lhs,
        WideUnsigned rhs) noexcept {
        return lhs.high > rhs.high ||
               (lhs.high == rhs.high && lhs.low > rhs.low);
    }

    // Outside the native-U64 sum range, floating point supplies only the
    // estimate; exact 128-bit integer comparisons correct the final raw value.
    [[nodiscard]] static NO_INLINE ValueType HypotFallback(U64 lhs,
                                                            U64 rhs) noexcept {
        constexpr U64 maximumRaw =
            static_cast<U64>(std::numeric_limits<I64>::max());
        if (lhs > maximumRaw || rhs > maximumRaw) {
            return ValueType::FromRaw(std::numeric_limits<I64>::max());
        }

        const WideUnsigned squaredLength = AddWide(SquareWide(lhs),
                                                   SquareWide(rhs));
        if (GreaterWide(squaredLength, SquareWide(maximumRaw))) {
            return ValueType::FromRaw(std::numeric_limits<I64>::max());
        }

        const Float64 length = std::hypot(static_cast<Float64>(lhs),
                                          static_cast<Float64>(rhs));
        U64 result = static_cast<U64>(length);
        if (result > maximumRaw) {
            result = maximumRaw;
        }
        while (GreaterWide(SquareWide(result), squaredLength)) {
            --result;
        }
        while (result < maximumRaw) {
            const U64 next = result + 1;
            if (GreaterWide(SquareWide(next), squaredLength)) {
                break;
            }
            result = next;
        }
        return ValueType::FromRaw(static_cast<I64>(result));
    }

    [[nodiscard]] static FORCE_INLINE I64 ApplySign(U64 magnitude,
                                                     bool negative) noexcept {
        constexpr U64 negativeLimit = U64{1} << 63;
        if (negative) {
            if (magnitude >= negativeLimit) {
                return std::numeric_limits<I64>::min();
            }
            return -static_cast<I64>(magnitude);
        }
        if (magnitude >= negativeLimit) {
            return std::numeric_limits<I64>::max();
        }
        return static_cast<I64>(magnitude);
    }
};

[[nodiscard]] FORCE_INLINE Vec2 operator*(Vec2::ValueType scalar,
                                          const Vec2& value) noexcept {
    return value * scalar;
}

template<typename Integer,
         typename std::enable_if<
             std::is_integral<Integer>::value, int>::type = 0>
[[nodiscard]] FORCE_INLINE Vec2 operator*(Integer scalar,
                                          const Vec2& value) noexcept {
    return value * scalar;
}

template<typename Floating,
         typename std::enable_if<
             std::is_floating_point<Floating>::value, int>::type = 0>
Vec2 operator*(Floating, const Vec2&) = delete;

static_assert(sizeof(IVec2) == sizeof(I32) * 2, "IVec2 must contain no padding");
static_assert(sizeof(Vec2) == sizeof(I64) * 2, "Vec2 must contain no padding");
static_assert(std::is_trivially_copyable<IVec2>::value,
              "IVec2 must remain trivially copyable");
static_assert(std::is_trivially_copyable<Vec2>::value,
              "Vec2 must remain trivially copyable");
static_assert(std::is_standard_layout<IVec2>::value,
              "IVec2 must remain standard-layout");
static_assert(std::is_standard_layout<Vec2>::value,
              "Vec2 must remain standard-layout");
