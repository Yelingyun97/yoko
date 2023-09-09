#include "Xml.h"

#include <fstream>
#include <iostream>

static const int detail_len = 60;

#define THROW_ERROR(error_info, error_detail) \
    do{ \
    std::string info = "parse error in "; \
    std::string file_pos = __FILE__; \
    file_pos.append(":"); \
    file_pos.append(std::to_string(__LINE__)); \
    info += file_pos; \
    info += ", "; \
    info += (error_info); \
    info += "\ndetail:"; \
    info += (error_detail); \
    throw std::logic_error(info); \
}while(false)

namespace yoko {

void Xml::loadFile(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("file not exist");
    }
    m_str = std::move(std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()));
    m_idx = 0;
    trim();
    m_len = m_str.size();
    parse();
}

void Xml::loadString(const std::string &str) {
    m_str = str;
    trim();
    m_idx = 0;
    m_len = m_str.size();
    parse();
}

void Xml::parse() {
    get_c();

    // 声明只会在文档开头，且就一个
    if (m_str.compare(m_idx, 5, "<?xml") == 0) {
        parse_decl();
    }

    // 根结点只有一个,用flag标记
    bool flag = true;
    while (m_idx < m_len) {
        get_c();
        // 解析注释
        if (!m_str.compare(m_idx, 4, "<!--")) {
            parse_comment();
            continue;
        }

        // 解析根结点
        if (flag && m_idx + 1 < m_len && m_str[m_idx + 1]) {
            m_root = parse_node();
            flag = false;
            continue;
        }

        // 其他不符合情况就抛异常
        THROW_ERROR("format error", m_str.substr(m_idx, detail_len));
    }
}

// <?xml version="1.0" encoding="utf-8"?>
void Xml::parse_decl() { 
    m_idx += 5;
    // 暂时不处理
    while (m_idx < m_str.size() && m_str[m_idx] != '?') {
        ++m_idx;
    }

    if (m_idx + 1 >= m_str.size() || m_str[m_idx + 1] != '>') {
        THROW_ERROR("format error", m_str.substr(m_idx, detail_len));
    }
    m_idx += 2;
}

// <!-- -->
void Xml::parse_comment() {
    m_idx += 4;
    std::string comment;
    std::size_t next_idx = m_str.find("--", m_idx);
    if (next_idx != std::string::npos) {;
        if (next_idx + 2 < m_str.size() && m_str[next_idx + 2] == '>') {
            comment = m_str.substr(m_idx, next_idx - m_idx);
            m_idx = next_idx + 3;
            // 将注释加入结点。。。
            return;
        }
    }

    THROW_ERROR("format error", m_str.substr(m_idx, detail_len));
}

Node Xml::parse_node() {
    Node node;
    char ch = get_c();
    if (ch != '<') {
        THROW_ERROR("format error", m_str.substr(m_idx, detail_len));
    }
    ++m_idx;
    // 解析标签名字，成功时m_idx停留在空白符、'/'或者'>'上
    int ret = parse_name(node);
    if (!ret) {
        THROW_ERROR("parse name error", m_str.substr(m_idx, detail_len));
    }
    
    // 解析标签名字，成功时m_idx停留在'/'或者'>'上
    ret = parse_attr(node);
    if (!ret) {
        THROW_ERROR("parse attribution error", m_str.substr(m_idx, detail_len));
    }

    // 说明不包含文本和子标签
    if (!m_str.compare(m_idx, 2, "/>")) {
        m_idx += 2;
        return node;
    }
    
    // 解析文本（其中会包含子标签、文本和注释）
    if (m_str[m_idx] == '>') {
        ++m_idx;
        ret = parse_text(node);
        if (ret) {
            return node;
        }
    }
    THROW_ERROR("parse text error", m_str.substr(m_idx, detail_len));
}

// 解析完标签名，m_idx处在空白字符、'/'或者'>'上，其他情况都返回false
bool Xml::parse_name(Node &node) {
    std::string name;
    while (m_idx < m_len && chk_c(m_str[m_idx])) {
        name.push_back(m_str[m_idx]);
        ++m_idx;
    }

    // m_idx超过字符串大小或者当前的字符不为空白字符、'/'和'>'
    if (m_idx >= m_len || (!isspace(m_str[m_idx]) && m_str[m_idx] != '>' && m_str[m_idx] != '/')) {
        return false;
    }
    node.set_name(name);
    return true;
}

// 解析完属性，m_idx位于在'/'或者'>'时返回true，其他情况返回false
bool Xml::parse_attr(Node &node) {
    char ch = get_c();
    while (chk_1_c(ch)) {
        std::string k, v;

        while (m_idx < m_str.size() && chk_c(m_str[m_idx])) {
            k.push_back(m_str[m_idx]);
            ++m_idx;
        }

        if (get_c() != '=') {
            return false;
        }
        m_idx++;

        if (get_c() != '"') {
            return false;
        }
        m_idx++;

        while (m_idx < m_str.size() && m_str[m_idx] != '"') {
            v.push_back(m_str[m_idx]);
            ++m_idx;
        }

        if (m_idx >= m_len) {
            return false;
        }

        // 过滤掉引号
        ++m_idx;
        node[k] = v;
        ch = get_c();
    }

    if (ch == '>' || ch == '/') {
        return true;
    }
    return false;
}

// 解析文本, m_idx位于字符'>'的下一个位置true，否则返回false
bool Xml::parse_text(Node &node) {
    std::string text;
    while (m_idx < m_len) {
        while (m_idx < m_len && m_str[m_idx] != '<') {
            text.push_back(m_str[m_idx]);
            ++m_idx;
        }
        
        // 根据是结束标签、注释或子标签分情况处理
        if (!m_str.compare(m_idx, 2, "</")) {
            m_idx += 2;
            std::string name;
            
            // 这里不需要检查标签名是否合法，因为最后都是要和相对应标签的合法名字比较
            while (m_idx < m_len && !isspace(m_str[m_idx]) && m_str[m_idx] != '>') {
                name.push_back(m_str[m_idx]);
                ++m_idx;
            }
            char ch = get_c();
            if (ch == '>' && !name.compare(node.get_name())) {
                node.set_text(text);
                ++m_idx;
                return true;
            }
            return false;
        } else if (!m_str.compare(m_idx, 4, "<!--")) {
            parse_comment();
        } else {
            // 添加子标签结点，解析失败parse_node()会抛异常
            node.add_node(parse_node());
        }
    }
    // 正常结束循环只能说字符串不够解析了，返回false
    return false;
}

void Xml::trim() {
    int i = m_str.size() - 1;
    while (i >= 0 && isspace(m_str[i])) {
        --i;
    }
    m_str.resize(i + 1);
}

bool Xml::chk_1_c(char c) const {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c== '_') {
        return true;
    }
    return false;
}

bool Xml::chk_c(char c) const {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
        return true;
    }
    return false;
}

char Xml::get_c() {
    while (m_idx < m_len && isspace(m_str[m_idx])) {
        ++m_idx;
    }
    if (m_idx >= m_len) {
        THROW_ERROR("document incomplete", m_str.substr(m_idx, detail_len));
    }
    return m_str[m_idx];
}

std::string Xml::purify_text(const std::string &text) {
    std::string pt;
    int l = 0, r = text.size() - 1;
    while (l <= r && isspace(text[l])) {
        l++;
    }

    while (l <= r && isspace(text[r])) {
        r--;
    }

    bool flag = true;
    while (l <= r) {
        if (!isspace(text[l])) {
            pt.push_back(text[l]);
            flag = true;
        } else if (flag) {
            pt.push_back(' ');
            flag = false;
        }
        ++l;
    }
    return pt;
}

void Xml::print() {
    std::cout << print(m_root, 0);
}

std::string Xml::print(Node &node, int level) {
    std::string text = purify_text(node.get_text());
    std::stringstream ss;
    int cnt = 4 * level;
    ss << std::string(cnt, ' ') << "<" << node.get_name();
    Node::attrs atts = node.get_all_attrs();
    for (const auto &it : atts) {
        ss << " " << it.first << "=\"" << it.second << '"';
    }
    if (text.empty() && node.empty()) {
        ss << "/>\n";
    } else if (node.empty()) {
        ss << ">" << text << "</" << node.get_name() << ">\n";
    } else if (text.empty()) {
        ss << ">" <<"\n";
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss << print(*it, level + 1);
        }
        ss << std::string(cnt, ' ') << "</" << node.get_name() << ">\n";
    } else {
        ss << ">" <<"\n" << std::string(4 + cnt, ' ') << text << "\n";
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss << print(*it, level + 1);
        }
        ss << std::string(cnt, ' ') << "</" << node.get_name() << ">\n";
    }

    return ss.str();
}

}