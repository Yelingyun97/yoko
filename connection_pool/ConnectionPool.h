#pragma once

#include "Connection.h"

#include <string>
#include <queue>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#define LOG(str) \
	std::cout << __FILE__ << ":" << __LINE__ << " " << \
	__TIMESTAMP__ << " : " << str << std::endl;

namespace yoko
{

/**
 * 连接池类
 */
class ConnectionPool {
public:
    static ConnectionPool *instance();

    std::shared_ptr<Connection> getConnection();
private:
    ConnectionPool();
    bool loadConfig();
    void createConnectionThread();
    void scannerConnectionThread();

    std::string ip_; // 连接主机IP
    uint16_t port_;  // 连接端口号
    std::string dbname_;    // 数据库名称
    std::string username_;   // mysql用户名
    std::string password_; // mysql用户密码
    int initSize_;   // 初始连接数
    int maxSize_;    // 最大连接数
    int maxIdleTime_;   // 最大空闲时间
    int connectionTimeout_;    // 连接超时时间

    std::queue<Connection*> connections_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic_int connectionNum_;     // 连接池数量
};

} // namespace yoko
