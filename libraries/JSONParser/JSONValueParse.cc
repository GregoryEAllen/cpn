#include "JSONValueParse.h"
namespace JSON {
    shared_ptr<Value> ValueParser::Get() {
        shared_ptr<Value> ret = stack.top();
        stack.pop();
        return ret;
    }

    bool ValueParser::ArrayBegin() {
        stack.push(shared_ptr<Value>(new ArrayValue));
        return true;
    }

    bool ValueParser::ArrayEnd() {
        while (true) {
            shared_ptr<Value> top = stack.top();
            shared_ptr<ArrayValue> a = dynamic_pointer_cast<ArrayValue>(top);
            if (a && !a->done) {
                a->done = true;
                a->child = a->next;
                a->next.reset();
                return true;
            }
            stack.pop();
            stack.top()->next = top;
        }
    }

    bool ValueParser::ObjectBegin() {
        stack.push(shared_ptr<Value>(new ObjectValue));
        return true;
    }

    bool ValueParser::ObjectEnd() {
        return false;
    }
    bool ValueParser::Integer(int64_t value) {
        stack.push(shared_ptr<Value>(new IntegerValue(value)));
        return true;
    }

    bool ValueParser::Float(double value) {
        stack.push(shared_ptr<Value>(new FloatValue(value)));
        return true;
    }

    bool ValueParser::String(const std::string &str) {
        stack.push(shared_ptr<Value>(new StringValue(str)));
        return true;
    }

    bool ValueParser::Null() {
        stack.push(shared_ptr<Value>(new NullValue));
        return true;
    }

    bool ValueParser::True() {
        stack.push(shared_ptr<Value>(new TrueValue));
        return true;
    }

    bool ValueParser::False() {
        stack.push(shared_ptr<Value>(new FalseValue));
        return true;
    }

    bool ValueParser::Key(const std::string &str) {
        stack.push(shared_ptr<Value>(new KeyValue(str)));
        return true;
    }
}
