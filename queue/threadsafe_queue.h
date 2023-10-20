#include <memory>
#include <mutex>
#include <condition_variable>

namespace yoko
{

// =======================C++并发编程实战第6章的细粒度安全队列==========================
// push和try_pop几乎可以并发执行
// push时，为数据在堆上分配内存的时候是在锁外完成，加锁后只是移动指针，效率更高
// 并发程度高、异常安全
template <typename T>
class threadsafe_queue {
public:
    threadsafe_queue()
        : head_(new node)
        , tail_(head_.get()) {}
    
    threadsafe_queue(const threadsafe_queue &) = delete;
    threadsafe_queue *operator=(const threadsafe_queue &) = delete;
    
    std::shared_ptr<T> wait_and_pop() {
        const std::unique_ptr<node> old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T &val) {
        // 这里接收old_head应该是将其析构删除放在解锁后完成
        const std::unique_ptr<node> old_head = wait_pop_head(val);
    }

    // 只有加锁会抛异常，只有加锁后才会改数据，所以try_pop异常安全
    std::shared_ptr<T> try_pop() {
        // 这里用pop_head的好处是让old_head在锁外析构，析构的开销也高
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }

    bool try_pop(T &val) {
        const std::unique_ptr<node> old_head = try_pop_head(val);
        return old_head;
    }

    // 有两次在堆上new数据可能会抛异常，但是由于是智能指针管理，
    // 不管谁抛异常，都会自动释放内存，剩下的就是加锁可能会抛，因此也是异常安全。
    void push(T val) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(val)));
        std::unique_ptr<node> p(new node);
        node *const new_tail = p.get();
        {
            std::lock_guard<std::mutex> lock(tail_mutex_);
            tail_->data = new_data;
            tail_->next = std::move(p);
            tail_ = new_tail;
        }
        condv_.notify_one();
    }

    bool empty() {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        return head_.get() == get_tail();
    }
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    node *get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex_);
        return tail_;
    }

    std::unique_ptr<node> pop_head() {
        std::unique_ptr<node> old_head = std::move(head_);
        head_ = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data() {
        std::unique_lock<std::mutex> head_lock(head_mutex_);
        condv_.wait(head_lock, [this] { return head_.get() != get_tail(); });
        return std::move(head_lock);
    }

    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }

    std::unique_ptr<node> wait_pop_head(T &val) {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        val = std::move(*head_->data);  // 可能抛异常，取决于实例化的T类型，因此要在移除结点前完成
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head() {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        if (head_.get() == get_tail()) {
            return std::unique_ptr<node>();
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T &val) {
        std::lock_guard<std::mutex> head_lock(head_mutex_);
        if (head_.get() == get_tail()) {
            return std::unique_ptr<node>();
        }
        val = std::move(*head_->data);  // 可能抛异常，取决于实例化的T类型，因此要在移除结点前完成
        return pop_head();
    }

    std::mutex head_mutex_;
    std::mutex tail_mutex_;
    std::unique_ptr<node> head_;
    node *tail_;    // 不能使用unique_ptr保存尾结点，次尾结点的next也是unique_ptr
    std::condition_variable condv_;
};

} // namespace yoko
