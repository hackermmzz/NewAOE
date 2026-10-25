#pragma once

#include "Public/type/type.h"
#include "Public/util/util.h"

#include <boost/lexical_cast.hpp>

#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

// 保存一份文本，并按需要将它转换为项目使用的基础类型。
// 转换失败时，boost::lexical_cast 会抛出 boost::bad_lexical_cast；
// I8/U8 超出取值范围时会抛出 std::out_of_range。
class Cast final {
public:
    Cast() = default;

    Cast(const char *value) : data(value == nullptr ? "" : value) {}
    Cast(const std::string &value) : data(value) {}
    Cast(std::string &&value) noexcept : data(std::move(value)) {}

    Cast(const Cast &) = default;
    Cast(Cast &&) noexcept = default;
    Cast &operator=(const Cast &) = default;
    Cast &operator=(Cast &&) noexcept = default;

    Cast &operator=(const char *value) {
        data = value == nullptr ? "" : value;
        return *this;
    }

    Cast &operator=(const std::string &value) {
        data = value;
        return *this;
    }

    Cast &operator=(std::string &&value) noexcept {
        data = std::move(value);
        return *this;
    }

    FORCE_INLINE I8 ToI8() const {
        const I16 value = boost::lexical_cast<I16>(data);
        if (value < static_cast<I16>(std::numeric_limits<I8>::min()) ||
            value > static_cast<I16>(std::numeric_limits<I8>::max())) {
            throw std::out_of_range("Cast::ToI8");
        }
        return static_cast<I8>(value);
    }

    FORCE_INLINE I16 ToI16() const { return boost::lexical_cast<I16>(data); }
    FORCE_INLINE I32 ToI32() const { return boost::lexical_cast<I32>(data); }
    FORCE_INLINE I64 ToI64() const { return boost::lexical_cast<I64>(data); }

    FORCE_INLINE U8 ToU8() const {
        const U16 value = boost::lexical_cast<U16>(data);
        if (value > static_cast<U16>(std::numeric_limits<U8>::max())) {
            throw std::out_of_range("Cast::ToU8");
        }
        return static_cast<U8>(value);
    }

    FORCE_INLINE U16 ToU16() const { return boost::lexical_cast<U16>(data); }
    FORCE_INLINE U32 ToU32() const { return boost::lexical_cast<U32>(data); }
    FORCE_INLINE U64 ToU64() const { return boost::lexical_cast<U64>(data); }
    FORCE_INLINE Byte ToByte() const { return ToU8(); }

    FORCE_INLINE Float32 ToFloat32() const {
        return boost::lexical_cast<Float32>(data);
    }

    FORCE_INLINE Float64 ToFloat64() const {
        return boost::lexical_cast<Float64>(data);
    }

    FORCE_INLINE bool ToBool() const {
        if (data == "true" || data == "1") {
            return true;
        }
        if (data == "false" || data == "0") {
            return false;
        }
        throw boost::bad_lexical_cast(typeid(std::string), typeid(bool));
    }

    FORCE_INLINE std::string ToString() const { return data; }
    FORCE_INLINE const std::string &String() const noexcept { return data; }
    FORCE_INLINE const char *CStr() const noexcept { return data.c_str(); }
    FORCE_INLINE bool Empty() const noexcept { return data.empty(); }

    template<typename T>
    FORCE_INLINE T To() const {
        if constexpr (std::is_same<T, I8>::value) {
            return ToI8();
        } else if constexpr (std::is_same<T, U8>::value) {
            return ToU8();
        } else if constexpr (std::is_same<T, bool>::value) {
            return ToBool();
        } else {
            return boost::lexical_cast<T>(data);
        }
    }

private:
    std::string data;
};
