# xml解析器

这是一个简易的xml解析器，能够处理一些简单的xml格式文件，一些细节上的东西没能做出来，但xml解析器最基本的解析标签名、属性和文本都实现了。为了避免自己太过纠结在xml的各种奇奇怪怪的格式，我对该程序能处理的xml文件格式做了一些要求，主要是简化了格式，这样实现起来相对容易。

## xml格式


### 标签名

* 紧挨着'<'，不能包含空白字符，以字母或下划线开头；
* 其他部分可以包含字母、数字以及下划线。

### 属性

* 属性名的命名规则和标签名一样；
* 属性名和值中间用=相连，=左右两边可以有空白符；
* 属性值用引号包围，其中的字符没有限制；
* 属性名不能相同。(没实现，目前应该是同属姓名会覆盖)

### 文本

* 不包含单双引号、左右尖括号和'&'；（没实现，后来都忘了）
* 不跳过任何空白字符；
* 被子标签隔开的文本一起连接起来作为一个整体。

### 注释

* 以"<!--"开始，"--">结束；
* 注释的内容不能包括"--"。

### 声明

* 只有一个，只能出现在文件开头，以"<?xml"开始，"?>"结尾。（目前没对其中的属性进行解析，格式对就忽略了）

### 其他

* 成对出现的标签的匹配规则和括号的匹配规则一样；
* 成对出现的第二个标签以"</"开头，加标签名，后面可以有若干空白字符，解析时被忽略，最后以">"收尾；
* 单标签以"/>"结尾；
* 标签名和属性以及属性之间用空白字符隔开；
* 根结点就一个。

## 实现细节


在我的实现上，每个标签由一个Node的类来存储。

### Node类

```c++
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
```



### 整体思路

> 1. 先处理xml文件的声明；
> 2. 循环解析注释和根结点，xml文件的根结点只能有一个，只解析一次；
> 3. 其他情况抛出异常。

```c++
void Xml::parse() {
    // 过滤空白符，将m_idx移动到第一个字符并返回，下标越界抛异常
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
```



### 结点处理思路

> 1. 先解析标签名，出错抛出异常；
> 2. 循环解析标签属性，出错抛出异常；
> 3. 解析文本，文本中会遇到子标签（会递归调用该函数进行解析）和注释，出错抛出异常，成功时返回node。

```c++
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
```

### 处理文本思路

处理标签名和属性相对来说容易，这里就讲下处理文本的逻辑。

> * 在遇到'<'前的所有字符（包括空白符）作为文本；
> * 遇到'<'后判断之后的是注释、子标签还是结束标签，判断依据下一个字符，按下面说的情况处理：
>   * 如果是注释，则调用解析注释的函数pare_comment()；
>   * 如果是子标签，则递归调用parse_node()，将返回的标签结点添加进当前标签的结点；
>   * 如果是结束标签，就判断其和起始标签的名字是否相等，相等就返回结点。
> * 循环上述过程过程，直到字符解析完毕或者解析到结束标签时返回。

```c++
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
```

## 测试效果

***

最后放个测试用的文本的处理结果吧，这是处理的文件：

```xml
<?xml version="1.0" encoding="uft-8"?>
<workflow>
    <work    name="1" switch = "on" >
        <!-- comment -->
        <plugin name="echoplugin.so"  switch  ="on"  />
        <leaf>yoko</leaf>
        <!-- comment -->
    </work  >
    xxxx
    <!-- xxxxxx-->
</workflow>
<!-- comment -->

```

这是解析完用打印出来的结果（文本进行过去前后空白字符，连续空白字符替换为一个空格的处理，输出格式还进行了处理，不然打印出来很乱）：

> <workflow>
>     xxxx
>     <work name="1" switch="on">
>         <plugin name="echoplugin.so" switch="on"/>
>         <leaf>yoko</leaf>
>     </work>
> </workflow>

## 结尾


xml解析器的实现很大一部分参考了大佬[L_B_](https://github.com/ACking-you/MyUtil "L_B_的github")的代码，B站还有他的视频教程[C++手写xml解析器](https://www.bilibili.com/video/BV1Md4y1m78g/?spm_id_from=333.999.list.card_archive.click&vd_source=a36a154d77f1815c31c413047af1201c "视频教程")。总之算是锻炼了下动手能力吧，虽然程序的整体逻辑开起来不是很复杂，但是具体细节实现起来还是很麻烦的。因为自己是边写边想，找bug时自己都理不清楚了，后来有重新写了一遍，果然还是要想清楚后再动手。最后，程序还有很多不足，甚至会有我不知道的bug，但是目前就这样吧，毕竟要学的东西还很多。



