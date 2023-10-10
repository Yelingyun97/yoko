#include "ConnectionPool.h"

#include <thread>
#include <functional>

using namespace yoko;

ConnectionPool *ConnectionPool::instance() {
    static ConnectionPool pool;
    return &pool;
}

// 可以用自己写的xml解析器读配置文件
bool ConnectionPool::loadConfig() {
    FILE *fp = fopen("mysql.conf", "r");
    if (fp == nullptr) {
        LOG("mysql.conf file is not exist!");
    }

    while (!feof(fp)) {
        char buf[1024] = {0};
        fgets(buf, 1024, fp);
        std::string line(buf);
        size_t idx = line.find('=', 0);
        if (idx == std::string::npos) {
            return false;
        }

        size_t end = line.find('\n', idx);  // TODO:这里有问题，最后一行不一定包括'\n'
        std::string key = line.substr(0, idx);
        std::string value = line.substr(idx + 1, end - idx - 1);

        if (key == "ip") {
			ip_ = value;
		} else if (key == "port") {
			port_ = atoi(value.c_str());
		} else if (key == "username") {
			username_ = value;
		} else if (key == "password") {
			password_ = value;
		} else if (key == "db") {
			dbname_ = value;
		} else if (key == "initSize") {
			initSize_ = atoi(value.c_str());
		} else if (key == "maxSize") {
			maxSize_ = atoi(value.c_str());
		} else if (key == "maxIdleTime") {
			maxIdleTime_ = atoi(value.c_str());
		} else if (key == "connectionTimeOut") {
			connectionTimeout_ = atoi(value.c_str());
		}
    }
    return true;
}

ConnectionPool::ConnectionPool() {
    // 读配置文件
    if (!loadConfig()) {
        return;
    }

    // 初始化连接数
    for (int i = 0; i < initSize_; ++i) {
        Connection *conn = new Connection();
        if (conn->connect(ip_, port_, username_, password_, dbname_)) {
            connections_.push(conn);
            ++connectionNum_;
        }
    }

    // 开启生产数据库连接的线程
    std::thread producer(std::bind(&ConnectionPool::createConnectionThread, this));
    producer.detach();

    // 开启定时扫描线程
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionThread, this));
    scanner.detach();
}

void ConnectionPool::createConnectionThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx_);    // TODO:感觉临界区有点大
        while (!connections_.empty()) {
            cv_.wait(lock);
        }
        
        if (connectionNum_ < maxSize_) {
            Connection *conn = new Connection();
            if (conn->connect(ip_, port_, username_, password_, dbname_)) {
                connections_.push(conn);
                ++connectionNum_;
            }
        }
        cv_.notify_one();
    }
}

// 获取连接
std::shared_ptr<Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mtx_);
    while (connections_.empty()) {
        if (cv_.wait_for(lock, std::chrono::microseconds(100)) == std::cv_status::timeout) {
            if (connections_.empty()) {
                LOG("获取链接超时");
                return nullptr;
            } 
        }
    }

    std::shared_ptr<Connection> sp(connections_.front(), [&](Connection *conn) {
        std::lock_guard<std::mutex> lock(mtx_);
        conn->refreshTime();
        connections_.push(conn);
    });
    connections_.pop();
    cv_.notify_all();   // TODO:感觉这样设计怪怪的
    return sp;
}

// 关闭空闲时间超过maxIdleTime的连接
void ConnectionPool::scannerConnectionThread() {
    while (true) {
        // 每经过maxIdleTime的时间检查一次
        std::this_thread::sleep_for(std::chrono::seconds(maxIdleTime_));
        std::unique_lock<std::mutex> lock(mtx_);
        while (connectionNum_ > initSize_) {
            // 只需查看队头元素就好，后加入队列的肯定空闲时间更短
            Connection *conn = connections_.front();
            if (conn->getAliveTime() < maxIdleTime_ * 1000) break;
            connections_.pop();
            --connectionNum_;
            delete conn;
        }
    }
}