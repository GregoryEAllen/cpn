#ifndef JSONTOVARIANT_H
#define JSONTOVARIANT_H
#pragma once
#include "JSONParser.h"
#include "Variant.h"
#include <stack>
class JSONToVariant : public JSON::Parser {
public:
    Variant Get() const { return stack.top(); }
protected:
    bool ArrayBegin();
    bool ArrayEnd();
    bool ObjectBegin();
    bool ObjectEnd();
    bool Integer(int64_t value);
    bool Float(double value);
    bool String(const std::string &str);
    bool Null();
    bool True();
    bool False();
    bool Key(const std::string &str);

    bool AddValue(Variant val);
    bool InObject();
    bool InArray();

    std::stack<std::string> keystack;
    typedef std::stack<Variant> Stack;
    Stack stack;
};
#endif
