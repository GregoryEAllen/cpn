
#include "Variant.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include "JSONToVariant.h"
#include "VariantToJSON.h"

void VariantTest();

int main (int argc, char **argv) __attribute__((weak));
int main(int argv, char** argc) {
    VariantTest();
    return 0;
}

void VariantTest() {
    Variant null;
    assert(!null);
    Variant v;
    v = 5;
    assert(v);
    assert(v.AsInt() == 5);
    assert(v.AsUnsigned() == 5u);
    assert(v.AsDouble() == 5.0);
    assert(v == Variant(5));
    assert(v == "5");
    Variant map;
    map["one"] = v;
    map["two"] = "two";
    Variant list;
    list[0] = 1;
    list[2] = "two";
    list[5] = map;
    list[4] = 123u;
    std::cout << list << std::endl;
    std::cout << v << std::endl;
    std::string text = "[1,2,3,4,{\"A\":2}, \"text\"]";
    text += "[\"doodle\", \"foo\", \"bar\"]";
    std::istringstream iss(text);
    JSONToVariant jv;
    iss >> jv;
    v = jv.Get();
    std::cout << v << std::endl;
    std::cout << PrettyJSON(v) << std::endl;
    v = v;
    assert(v.IsArray());
    assert(v[4].IsObject());
    assert(v[5].IsString());
    assert(v[4]["A"].IsNumber());
    assert(v[4]["b"].IsNull());
    assert(v[10].IsNull());
    const Variant cv = v;
    assert(cv.IsArray());
    assert(cv[4].IsObject());
    assert(cv[5].IsString());
    assert(cv[4]["A"].IsNumber());
    assert(cv[4]["b"].IsNull());
    assert(cv[10].IsNull());
    v = v[4];
    std::cout << v << std::endl;
    v = "\n";
    std::cout << v << std::endl;

    JSONToVariant jv2;
    iss >> jv2;
    assert(jv2.Done());
    v = jv2.Get();
    assert(v[1].AsString() == "foo");

    std::string failtext = "[\"garbage\" \n    [";
    std::cout << "Testing parser failure: " << failtext << std::endl;

    JSONToVariant p;
    std::istringstream iss2(failtext);
    iss2 >> p;
    assert(p.GetStatus() == JSON::Parser::ERROR);
    std::cout << "Stopped on line: " << p.GetLine() << " Column: " << p.GetColumn() << std::endl;
    std::string left;
    iss2 >> left;
    std::cout << "String left: " << left << std::endl;

}

