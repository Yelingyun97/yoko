#include <iostream>
#include <thread>
#include <atomic>

#include "threadsafe_queue.h"

using namespace yoko;

int main() {
    std::atomic<bool> go(false);
    threadsafe_queue<int> q;
    std::thread t1([&] {
        while (!go.load());
        for (int i = 0; i < 10000; ++i) {
            q.push(i);
        }
    });
    std::thread t2([&] {
        while (!go.load());
        for (int i = 0; i < 5000; ++i) {
            // auto v = q.try_pop();
            // if (v) std::cout << *v << "/";
            // (std::cout << *q.wait_and_pop() << " ").flush();
            int val;
            q.wait_and_pop(val);
            std::cout << val << " ";
        }
    });
    std::thread t3([&] {
        while (!go.load());
        for (int i = 0; i < 5000; ++i) {
            // auto v = q.try_pop();
            // if (v) std::cout << *v << " ";
            // (std::cout << *q.wait_and_pop() << " ").flush();
            int val;
            q.wait_and_pop(val);
            std::cout << val << " ";
        }
    });
    go.store(true);
    t1.join();
    t2.join();
    t3.join();
    std::cout << std::endl;
}