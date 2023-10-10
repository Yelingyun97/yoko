#pragma once

#include <mysql/mysql.h>
#include <string>
#include <ctime>

namespace yoko
{

/**
 * 封装mysql操作的类
 * 查询结果得到的MYSQL_RES的需要用户释放，不好用
 */
class Connection {
public:
    Connection();
    ~Connection();

    bool connect(std::string ip, uint16_t port, std::string user,
                std::string passwd, std::string db);
    bool update(std::string sql);
    MYSQL_RES *query(std::string sql);

    void refreshTime() { startTime_ = clock(); }
    clock_t getAliveTime() const { return clock() - startTime_; }
private:
    MYSQL *conn_;
    clock_t startTime_; // 开始空闲的时间
};

} // namespace yoko