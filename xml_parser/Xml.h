#ifndef __YOKO_PARSER_H__
#define __YOKO_PARSER_H__

#include <boost/optional.hpp>
#include <utility>
#include "Node.h"

namespace yoko {

class Xml {
public:
    void loadFile(const std::string &filename);
    void loadString(const std::string &str);
    Node get_root() const { return m_root; }
    void print();

private:
    void parse();
    void parse_decl();
    void parse_comment();
    Node parse_node();
    bool parse_name(Node &node);
    bool parse_attr(Node &node);
    bool parse_text(Node &node);
    std::string print(Node &node, int level);

    // 去掉文本前后的空白符，将多个连续空白符替换成一个' '
    std::string purify_text(const std::string &text);

    //去掉m_str后面的空白字符
    void trim();

    // 检查名字首字符是否合法
    bool chk_1_c(char c) const;

    // 检查名称字符是否合法
    bool chk_c(char c) const;

    // 获取第一个不为空白符的字符，如果下标越界则抛出异常
    char get_c();
private:
    Node m_root;
    std::string m_version;  // todo
    std::string m_encode;   // todo
    std::string m_str;
    std::size_t m_idx;
    std::size_t m_len;
};

}

#endif