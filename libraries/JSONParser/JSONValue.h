
#ifndef JSONVALUE_H
#define JSONVALUE_H
#pragma once
#include <string>
#include <stdint.h>
#include <tr1/memory>
namespace JSON {
    using std::tr1::shared_ptr;
    using std::tr1::weak_ptr;
    class Value {
    public:
        virtual ~Value() {}
        shared_ptr<Value> next;
        shared_ptr<Value> child;
    protected:
        Value() {}
    };
    class ObjectValue : public Value {
    public:
        ObjectValue() : done(false) {}
        bool done;
    };
    class ArrayValue : public Value {
    public:
        ArrayValue() : done(false) {}
        bool done;
    };
    class StringValue : public Value {
    public:
        StringValue(const std::string &s) : value(s) {}
        std::string value;
    };
    class KeyValue : public Value {
    public:
        KeyValue(const std::string &s) : value(s) {}
        std::string value;
    };
    class IntegerValue : public Value {
    public:
        IntegerValue(int64_t i) : value(i) {}
        int64_t value;
    };
    class FloatValue : public Value {
    public:
        FloatValue(double v) : value(v) {}
        double value;
    };
    class TrueValue : public Value {};
    class FalseValue : public Value {};
    class NullValue : public Value {};
}
#endif
