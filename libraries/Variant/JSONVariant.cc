
#include "JSONVariant.h"

bool JSONVariant::ArrayBegin() {
    stack.push(Variant(Variant::ArrayType));
    return true;
}

bool JSONVariant::ArrayEnd() {
    Variant v = stack.top();
    if (!v.IsArray()) return false;
    stack.pop();
    return AddValue(v);
}

bool JSONVariant::ObjectBegin() {
    stack.push(Variant(Variant::ObjectType));
    return true;
}

bool JSONVariant::ObjectEnd() {
    Variant v = stack.top();
    if (!v.IsObject()) return false;
    stack.pop();
    return AddValue(v);
}

bool JSONVariant::Integer(int64_t value) {
    Variant val = value;
    return AddValue(val);
}

bool JSONVariant::Float(double value) {
    Variant val = value;
    return AddValue(val);
}

bool JSONVariant::String(const std::string &str) {
    Variant val = str;
    return AddValue(val);
}

bool JSONVariant::Null() {
    return AddValue(Variant(Variant::NullType));
}

bool JSONVariant::True() {
    return AddValue(Variant(Variant::TrueType));
}

bool JSONVariant::False() {
    return AddValue(Variant(Variant::FalseType));
}

bool JSONVariant::Key(const std::string &str) {
    if (InObject()) {
        keystack.push(str);
        return true;
    }
    return false;
}

bool JSONVariant::AddValue(Variant val) {
    if (InObject()) {
        if (keystack.empty()) return false;
        stack.top().At(keystack.top()) = val;
        keystack.pop();
    } else if (InArray()) {
        stack.top().Append(val);
    } else {
        stack.push(val);
    }
    return true;
}

bool JSONVariant::InObject() {
    if (stack.empty()) return false;
    return stack.top().IsObject();
}

bool JSONVariant::InArray() {
    if (stack.empty()) return false;
    return stack.top().IsArray();
}

