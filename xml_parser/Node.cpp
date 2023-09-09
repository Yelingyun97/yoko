#include <stdexcept>

#include "Node.h"

namespace yoko {

boost::optional<std::string> Node::get_attr(const std::string &key) {
    if (m_attrs.count(key)) {
        return m_attrs[key];
    }
    return boost::none;
}

std::string Node::to_string() {
    std::stringstream ss;
    ss << "<" << m_name;

    for (const auto &attr : m_attrs) {
        ss << " " << attr.first << "=\"" << attr.second << '"';
    }

    if (m_text.empty() && m_childs.empty()) {
        ss << "/>";
    } else {
        ss << ">" << m_text;
        for (auto it = begin(); it != end(); ++it) {
            ss << it->to_string();
        }

        ss << "</" << m_name << ">";
    }
    return ss.str();
}

}