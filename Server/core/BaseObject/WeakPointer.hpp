#pragma once

#include "SafePointer.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

template<class T>
class WeakPtr
{
    template<class U>
    friend class WeakPtr;

public:
    using element_type = T;

    constexpr WeakPtr() noexcept : pointer_(nullptr), control_(nullptr) {}
    constexpr WeakPtr(std::nullptr_t) noexcept
        : pointer_(nullptr), control_(nullptr)
    {
    }

    WeakPtr(const WeakPtr& other) noexcept
        : pointer_(other.view()), control_(other.control_)
    {
        retain();
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    WeakPtr(const WeakPtr<U>& other) noexcept
        : pointer_(other.view()), control_(other.control_)
    {
        retain();
    }

    WeakPtr(WeakPtr&& other) noexcept
        : pointer_(other.view()), control_(other.control_)
    {
        other.pointer_ = nullptr;
        other.control_ = nullptr;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    WeakPtr(WeakPtr<U>&& other) noexcept
        : pointer_(other.view()), control_(other.control_)
    {
        other.pointer_ = nullptr;
        other.control_ = nullptr;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    WeakPtr(const SafePtr<U>& other) noexcept
        : pointer_(other.get()), control_(other.control_)
    {
        retain();
    }

    ~WeakPtr() noexcept
    {
        release();
    }

    WeakPtr& operator=(const WeakPtr& other) noexcept
    {
        if (this != &other) {
            WeakPtr(other).swap(*this);
        }
        return *this;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    WeakPtr& operator=(const WeakPtr<U>& other) noexcept
    {
        WeakPtr(other).swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept
    {
        WeakPtr(std::move(other)).swap(*this);
        return *this;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    WeakPtr& operator=(WeakPtr<U>&& other) noexcept
    {
        WeakPtr(std::move(other)).swap(*this);
        return *this;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    WeakPtr& operator=(const SafePtr<U>& other) noexcept
    {
        WeakPtr(other).swap(*this);
        return *this;
    }

    WeakPtr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    void reset() noexcept
    {
        WeakPtr().swap(*this);
    }

    void swap(WeakPtr& other) noexcept
    {
        using std::swap;
        swap(pointer_, other.pointer_);
        swap(control_, other.control_);
    }

    std::size_t use_count() const noexcept
    {
        return control_ == nullptr ? 0 : control_->referenceCount;
    }

    bool expired() const noexcept
    {
        return control_ == nullptr
            || control_->referenceCount == 0
            || control_->destroyed;
    }

    bool IsNull() const noexcept
    {
        return expired();
    }

    bool isNull() const noexcept
    {
        return IsNull();
    }

    bool IsDestroyed() const noexcept
    {
        return control_ != nullptr && control_->destroyed;
    }

    bool isDestroyed() const noexcept
    {
        return IsDestroyed();
    }

    explicit operator bool() const noexcept
    {
        return !expired();
    }

    SafePtr<T> lock() const noexcept
    {
        SafePtr<T> result;
        if (expired()) {
            return result;
        }

        // No compare/exchange loop is needed: this pointer is intentionally
        // single-threaded, so the validity check and increment cannot race.
        ++control_->referenceCount;
        result.pointer_ = pointer_;
        result.control_ = control_;
        return result;
    }

private:
    T* view() const noexcept
    {
        return expired() ? nullptr : pointer_;
    }

    void retain() noexcept
    {
        if (control_ != nullptr) {
            ++control_->weakReferenceCount;
        }
    }

    void release() noexcept
    {
        safe_ptr_detail::ControlBlock* control = control_;
        pointer_ = nullptr;
        control_ = nullptr;

        if (control == nullptr) {
            return;
        }

        --control->weakReferenceCount;
        if (control->weakReferenceCount == 0) {
            delete control;
        }
    }

    T* pointer_;
    safe_ptr_detail::ControlBlock* control_;
};

template<class T>
void swap(WeakPtr<T>& left, WeakPtr<T>& right) noexcept
{
    left.swap(right);
}
