#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

#include <utility>

namespace yoko {

// 独占指针
template <typename T>
class UniquePtr {
public:
    constexpr UniquePtr() = default;

    constexpr UniquePtr(nullptr_t) : UniquePtr() {}

    explicit UniquePtr(T *ptr) : ptr_(ptr) {}

    UniquePtr(const UniquePtr &) = delete;

    UniquePtr(UniquePtr &&rhs) noexcept : ptr_(rhs.release()) {}

    ~UniquePtr() noexcept { delete ptr_; }
    
    UniquePtr &operator=(const UniquePtr &) = delete;

    constexpr UniquePtr &operator=(nullptr_t) noexcept {
        reset();
        return *this;
    }
    // 移动赋值要将rhs中的指针置空，释放自己管理的内存后，再将指针指向rhs管理的内存
    UniquePtr &operator=(UniquePtr &&rhs) noexcept {
        reset(rhs.release());
        return *this;
    }

    T &operator*() const { return *ptr_; }

    T *operator->() const { return ptr_; }
    explicit operator bool() const {
        return static_cast<bool>(ptr_);
    }

    // 返回裸指针
    T *get() const { return ptr_; }   

    // 将ptr_置空，然后返回指向该内存的裸指针
    T *release() {
        return std::exchange(ptr_, nullptr);
    }

    // 释放自己管理的内存，然后指向新的内存块
    void reset(T *ptr = nullptr) {
        delete std::exchange(ptr_, ptr);
    }

    void swap(UniquePtr &rhs) {
        std::swap(ptr_, rhs.ptr_);
    }

private:
    T *ptr_{nullptr};
};

template <class T, class... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

}

#endif