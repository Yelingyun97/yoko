#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <optional>

namespace yoko
{

/**
 * 线程安全的队列(无大小限制)
 * 需要c++17支持
 */
template <class T>
class Queue {
public:
    void push(const T &val) {
        emplace(val);
    }

    void push(T &&val) {
        emplace(std::move(val));
    }

    template <class ...Args>
    void emplace(Args &&...args) {
        std::lock_guard lock(mtx_);
        q_.emplace(std::forward<Args>(args)...);
        cv_.notify_one();
    }

    T pop() {
        std::unique_lock lock(mtx_);
        cv_.wait(lock, [this] { return !q_.empty(); });
        assert(!q_.empty());
        T t(std::move_if_noexcept(q_.front()));
        q_.pop();
        return t;
    }

    std::optional<T> try_pop() {
        std::lock_guard lock(mtx_);
        if (q_.empty()) return {};

        std::optional<T> t(std::move_if_noexcept(q_.front()));
        q_.pop();
        return t;
    }

private:
    std::queue<T> q_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

} // namespace yoko