#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace safe_ptr_detail {

// SafePtr is intentionally single-threaded, so the reference count is a
// normal integer rather than an atomic integer.
class ControlBlock
{
public:
    ControlBlock() noexcept
        : referenceCount(1), weakReferenceCount(1), destroyed(false)
    {
    }
    virtual ~ControlBlock() = default;

    virtual void deleteManagedObject() noexcept = 0;

    std::size_t referenceCount;
    // Includes one implicit weak reference while referenceCount is non-zero.
    // This keeps the control block alive while the managed object's destructor
    // releases WeakPtr members that point back to the same object.
    std::size_t weakReferenceCount;
    bool destroyed;
};

template<class U>
class PointerControlBlock final : public ControlBlock
{
    static_assert(sizeof(U) > 0, "SafePtr requires a complete object type");

public:
    explicit PointerControlBlock(U* pointer) noexcept : pointer_(pointer) {}

    void deleteManagedObject() noexcept override
    {
        // The storage is released only when the last SafePtr goes away.
        // Setting the flag first also keeps get() null during U's destructor.
        destroyed = true;
        U* pointer = pointer_;
        pointer_ = nullptr;
        delete pointer;
    }

private:
    U* pointer_;
};

} // namespace safe_ptr_detail

template<class T>
class WeakPtr;

template<class T>
class SafePtr
{
    template<class U>
    friend class SafePtr;
    template<class U>
    friend class WeakPtr;

public:
    using element_type = T;

    constexpr SafePtr() noexcept : pointer_(nullptr), control_(nullptr) {}
    constexpr SafePtr(std::nullptr_t) noexcept
        : pointer_(nullptr), control_(nullptr)
    {
    }

    template<
        class U,
        typename std::enable_if<
            std::is_convertible<U*, T*>::value && std::is_object<U>::value,
            int>::type = 0>
    explicit SafePtr(U* pointer) : pointer_(pointer), control_(nullptr)
    {
        static_assert(sizeof(U) > 0, "SafePtr requires a complete object type");

        if (pointer == nullptr) {
            return;
        }

        try {
            // Keep U here (instead of T) so deleting a derived object remains
            // correct even when T does not have a virtual destructor.
            control_ = new safe_ptr_detail::PointerControlBlock<U>(pointer);
        } catch (...) {
            delete pointer;
            pointer_ = nullptr;
            throw;
        }
    }

    SafePtr(const SafePtr& other) noexcept
        : pointer_(other.get()), control_(other.control_)
    {
        retain();
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    SafePtr(const SafePtr<U>& other) noexcept
        : pointer_(other.get()), control_(other.control_)
    {
        retain();
    }

    SafePtr(SafePtr&& other) noexcept
        : pointer_(other.get()), control_(other.control_)
    {
        other.pointer_ = nullptr;
        other.control_ = nullptr;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    SafePtr(SafePtr<U>&& other) noexcept
        : pointer_(other.get()), control_(other.control_)
    {
        other.pointer_ = nullptr;
        other.control_ = nullptr;
    }

    ~SafePtr() noexcept
    {
        release();
    }

    SafePtr& operator=(const SafePtr& other) noexcept
    {
        if (this != &other) {
            SafePtr(other).swap(*this);
        }
        return *this;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    SafePtr& operator=(const SafePtr<U>& other) noexcept
    {
        SafePtr(other).swap(*this);
        return *this;
    }

    SafePtr& operator=(SafePtr&& other) noexcept
    {
        SafePtr(std::move(other)).swap(*this);
        return *this;
    }

    template<
        class U,
        typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0>
    SafePtr& operator=(SafePtr<U>&& other) noexcept
    {
        SafePtr(std::move(other)).swap(*this);
        return *this;
    }

    SafePtr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    void reset() noexcept
    {
        SafePtr().swap(*this);
    }

    template<
        class U,
        typename std::enable_if<
            std::is_convertible<U*, T*>::value && std::is_object<U>::value,
            int>::type = 0>
    void reset(U* pointer)
    {
        SafePtr(pointer).swap(*this);
    }

    void swap(SafePtr& other) noexcept
    {
        using std::swap;
        swap(pointer_, other.pointer_);
        swap(control_, other.control_);
    }

    T* get() const noexcept
    {
        if (control_ == nullptr || control_->destroyed) {
            return nullptr;
        }
        return pointer_;
    }

    bool IsNull() const noexcept
    {
        return get() == nullptr;
    }

    // Qt-style spelling, kept alongside the requested IsNull spelling.
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

    std::size_t use_count() const noexcept
    {
        return control_ == nullptr ? 0 : control_->referenceCount;
    }

    bool unique() const noexcept
    {
        return use_count() == 1;
    }

    explicit operator bool() const noexcept
    {
        return !IsNull();
    }

    bool operator!() const noexcept
    {
        return IsNull();
    }

    template<class U = T>
    typename std::enable_if<!std::is_void<U>::value, U&>::type
    operator*() const noexcept
    {
        return *get();
    }

    T* operator->() const noexcept
    {
        return get();
    }

    // Marks the object as logically destroyed. The managed object remains
    // allocated until the final SafePtr releases the shared control block.
    // The raw pointer returned by get() must never be deleted directly.
    void destroy() noexcept
    {
        if (control_ != nullptr) {
            control_->destroyed = true;
        }
        pointer_ = nullptr;
    }

    void Destroy() noexcept
    {
        destroy();
    }

private:
    void retain() noexcept
    {
        if (control_ != nullptr) {
            ++control_->referenceCount;
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

        --control->referenceCount;
        if (control->referenceCount == 0) {
            control->deleteManagedObject();

            // Release the implicit weak reference owned by the strong side.
            --control->weakReferenceCount;
            if (control->weakReferenceCount == 0) {
                delete control;
            }
        }
    }

    T* pointer_;
    safe_ptr_detail::ControlBlock* control_;
};

template<class T>
void swap(SafePtr<T>& left, SafePtr<T>& right) noexcept
{
    left.swap(right);
}

template<class T, class U>
bool operator==(const SafePtr<T>& left, const SafePtr<U>& right) noexcept
{
    return left.get() == right.get();
}

template<class T, class U>
bool operator!=(const SafePtr<T>& left, const SafePtr<U>& right) noexcept
{
    return !(left == right);
}

template<class T>
bool operator==(const SafePtr<T>& pointer, std::nullptr_t) noexcept
{
    return pointer.IsNull();
}

template<class T>
bool operator==(std::nullptr_t, const SafePtr<T>& pointer) noexcept
{
    return pointer.IsNull();
}

template<class T>
bool operator!=(const SafePtr<T>& pointer, std::nullptr_t) noexcept
{
    return !pointer.IsNull();
}

template<class T>
bool operator!=(std::nullptr_t, const SafePtr<T>& pointer) noexcept
{
    return !pointer.IsNull();
}
