#include <iostream>
#include "../xml-parser/Node.h"
#include "../xml-parser/Xml.h"

using namespace yoko;
using namespace std;


int main() {
    Xml xml;
    xml.loadFile("test.xml");
    Node root = xml.get_root();
    // cout << root.to_string() << endl;
    xml.print();
    return 0;
}