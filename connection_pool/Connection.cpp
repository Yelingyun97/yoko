#include "Connection.h"

#include <iostream>

using namespace yoko;

// 初始化数据库连接
Connection::Connection() {
    conn_ = mysql_init(nullptr);
}

// 释放数据库连接资源
Connection::~Connection() {
    if (conn_ != nullptr) {
        mysql_close(conn_);
    }
}

// 连接数据库
bool Connection::connect(std::string ip, uint16_t port, std::string user,
            std::string passwd, std::string db) {
    MYSQL *p = mysql_real_connect(conn_, ip.c_str(), user.c_str(), passwd.c_str(),
                    db.c_str(), port, nullptr, 0);
    return p != nullptr;
}

// 增删改
bool Connection::update(std::string sql) {
    if (mysql_query(conn_, sql.c_str())) {
        std::cout << "查询失败:" << sql << std::endl;
        return false;
    }
    return true;
}

// 查询
MYSQL_RES *Connection::query(std::string sql) {
    if (mysql_query(conn_, sql.c_str())) {
        std::cout << "查询失败:" << sql << std::endl;
        return nullptr;
    }
    return mysql_use_result(conn_);
}