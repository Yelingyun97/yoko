#pragma once

#include <boost/circular_buffer.hpp>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <optional>

namespace yoko
{

/**
 * 线程安全的循环队列
 * 需要c++17支持、依赖boost库
 */
template <class T>
class CircularQueue {
public:
    CircularQueue(size_t capacity = 1024) : buf_(capacity) {}

    template <class U>
    void push(U &&val) {
        std::unique_lock lock(mtx_);
        notFull_.wait(lock, [this] { return !buf_.full(); });
        assert(!buf_.full());

        buf_.push_back(std::forward<U>(val));
        notEmpty_.notify_one();
    }

    T pop() {
        std::unique_lock lock(mtx_);
        notEmpty_.wait(lock, [this] { return !buf_.empty(); });
        assert(!buf_.empty());

        T t(std::move_if_noexcept(buf_.front()));
        buf_.pop_front();
        notFull_.notify_one();
        return t;
    }

    // 队列满了返回false
    template <class U>
    bool try_push(U &&val) {
        std::lock_guard lock(mtx_);
        if (buf_.full()) return false;

        buf_.push_back(std::forward<U>(val));
        notEmpty_.notify_one();
        return true;
    }
    
    // 队列为空返回null
    std::optional<T> try_pop() {
        std::lock_guard lock(mtx_);
        if (buf_.empty()) return std::nullopt;

        std::optional<T> t(std::move_if_noexcept(buf_.front()));
        buf_.pop_front();
        notFull_.notify_one();
        return t;
    }
private:
    boost::circular_buffer<T> buf_;
    std::mutex mtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
};

} // namespace yoko
