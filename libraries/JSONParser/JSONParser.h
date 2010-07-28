
#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP
#pragma once
#include "JSON_parser.h"
#include <string>
#include <stdint.h>
#include <iosfwd>
namespace JSON {
    class Parser {
    public:
        enum Status_t {
            OK,
            ERROR,
            DONE
        };
        Parser();
        virtual ~Parser();

        bool Parse(int c);

        unsigned Parse(const char *c, unsigned len);
        unsigned Parse(const std::string &str) { return Parse(str.data(), str.size()); }

        void ParseStream(std::istream &is);
        void ParseFile(FILE *f);
        bool ParseFile(const std::string &filename);

        Status_t GetStatus() const { return status; }
        bool Done() const { return status == DONE; }
        bool Error() const { return status == ERROR; }
        bool Ok() const { return status == OK; }
        unsigned GetLine() const { return line; }
        unsigned GetColumn() const { return column; }
        unsigned GetByteCount() const { return charcount; }
        virtual void Reset();
    protected:

        virtual bool ArrayBegin() = 0;
        virtual bool ArrayEnd() = 0;
        virtual bool ObjectBegin() = 0;
        virtual bool ObjectEnd() = 0;
        virtual bool Integer(int64_t value) = 0;
        virtual bool Float(double value) = 0;
        virtual bool String(const std::string &str) = 0;
        virtual bool Null() = 0;
        virtual bool True() = 0;
        virtual bool False() = 0;
        virtual bool Key(const std::string &str) = 0;

        static int StaticCallback(void *ctx, int type, const struct JSON_value_struct* value);
        int Callback(int type, const struct JSON_value_struct* value);

        JSON_parser parser;
        Status_t status;

        unsigned line;
        unsigned column;
        unsigned charcount;
        unsigned depth;
    private:
        Parser(const Parser  &);
        Parser & operator=(const Parser &);
        void AllocParser();
    };
    std::istream &operator>>(std::istream &is, Parser &p);
}


#endif
