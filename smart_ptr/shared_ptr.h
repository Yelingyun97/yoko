#ifndef __SHARED_PTR_H__
#define __SHARED_PTR_H__

#include <utility>

namespace yoko {

class RefCount {
public:
    RefCount() = default;
    int use_count() const noexcept { return count_; }
    void incRef() noexcept { ++count_; }
    int decRef() noexcept { return --count_; }

private:
    int count_{1};
};

// 使用裸指针初始化共享指针后就将内存所有权转移给共享指针管理，最好不要再使用该裸指针，
// 尤其注意不要用该裸指针初始化其他智能指针或者调用delete释放内存
template <typename T>
class SharedPtr {
public:
    constexpr SharedPtr() noexcept = default;

    constexpr SharedPtr(nullptr_t) : SharedPtr() {}

    explicit SharedPtr(T *ptr) : ptr_(ptr) {
        if (ptr) {
            rep_ = new RefCount();
        }
    }

    SharedPtr(const SharedPtr &rhs) noexcept : ptr_(rhs.ptr_), rep_(rhs.rep_) {
        if (rep_) {
            rep_->incRef();
        }
    }

    SharedPtr(SharedPtr &&rhs) noexcept : ptr_(rhs.ptr_), rep_(rhs.rep_) {
        rhs.ptr_ = nullptr;
        rhs.rep_ = nullptr;
    }

    ~SharedPtr() noexcept {
        if (rep_ && !rep_->decRef()) {
            delete ptr_;
            delete rep_;
        }
    }

    // 执行A = B时, 用B创建一个临时对象，B指向的内存引用计数+1，A和B交换管理的内存和引用计数，
    // 临时对象销毁，此时临时对象管理的是A原先管理的内存，引用计数-1
    SharedPtr &operator=(const SharedPtr &rhs) {
        SharedPtr(rhs).swap(*this);
        return *this;
    }

    // 执行A = B时, 用B创建一个临时对象，因为是调用移动构造，B指向的内存引用计数不变，A和B交换管理的内存和引用计数，
    // 临时对象销毁，此时临时对象管理的是A原先管理的内存，引用计数-1
    SharedPtr &operator=(SharedPtr &&rhs) {
        SharedPtr(std::move(rhs)).swap(*this);
        return *this;
    }

    void swap(SharedPtr &rhs) noexcept {
        std::swap(ptr_, rhs.ptr_);
        std::swap(rep_, rhs.rep_);
    }

    void reset(T *ptr = nullptr) {
        SharedPtr(ptr).swap(*this);
    }

    T *get() const noexcept { return ptr_; }
    T &operator*() const noexcept { return *ptr_; }
    T *operator->() const noexcept { return ptr_; }
    
    int use_count() const noexcept { return rep_ ? rep_->use_count() : 0; }
    bool unique() const noexcept { return use_count() == 1; }
    explicit operator bool() const noexcept { return static_cast<bool>(ptr_); }

private:
    T *ptr_{ nullptr };
    RefCount *rep_{ nullptr };
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward(args)...));
}

}

#endif