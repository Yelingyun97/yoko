#ifndef __YOKO_NODE_H__
#define __YOKO_NODE_H__

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <boost/optional.hpp>

namespace yoko {

class Node {
public:
    typedef std::vector<Node>::iterator iterator;
    typedef std::map<std::string, std::string> attrs;

    void set_name(const std::string &name) { m_name = name; }
    std::string get_name() const { return m_name; }

    void set_text(const std::string &text) { m_text = text; }
    std::string get_text() const { return m_text; }


    boost::optional<std::string> get_attr(const std::string &key);
    attrs get_all_attrs() const { return m_attrs; }
    std::string &operator[] (const std::string &key) { return m_attrs[key]; }

    iterator begin() { return m_childs.begin(); }
    iterator end() { return m_childs.end(); }
    bool empty() const { return m_childs.empty(); }
    void add_node(const Node &node) { m_childs.push_back(node); }

    std::string to_string();
    void print_format();

private:
    std::string m_name;
    std::string m_text;
    std::map<std::string, std::string> m_attrs;
    std::vector<Node> m_childs;
};

}

#endif