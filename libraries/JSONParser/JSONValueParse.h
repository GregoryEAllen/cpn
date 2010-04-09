
#pragma once
#include "JSONParser.h"
#include "JSONValue.h"
#include <stack>
namespace JSON {
    using std::tr1::dynamic_pointer_cast;
    class ValueParser : public Parser {
    public:
        shared_ptr<Value> Get();
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

        typedef std::stack< shared_ptr< Value > > Stack;
        Stack stack;
    };
}
