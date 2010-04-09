
#include "Variant.h"
#include <cassert>
#include <iostream>
#include "VariantJSON.h"
#include "JSONVariant.h"
#include <sstream>

void VariantTest();

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
    std::istringstream iss(text);
    JSONVariant jv;
    iss >> jv;
    v = jv.Get();
    //v = Variant::FromJSON(text);
    std::cout << v << std::endl;
    std::cout << PrettyJSON(v) << std::endl;
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
    v = "\n";
    std::cout << v;
}

