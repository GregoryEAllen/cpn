
#include "JSONParser.h"
#include <istream>

namespace JSON {
    int Parser::StaticCallback(void *ctx, int type, const struct JSON_value_struct* value) {
        Parser *parser = (Parser*)ctx;
        return parser->Callback(type, value);
    }


    Parser::Parser()
        : status(OK),
        line(1),
        column(0),
        charcount(0),
        depth(0)
    {
        JSON_config config;
        init_JSON_config(&config);
        config.callback = StaticCallback;
        config.callback_ctx = this;
        config.depth = -1;
        config.allow_comments = 1;
        parser = new_JSON_parser(&config);
    }

    Parser::~Parser() {
        delete_JSON_parser(parser);
    }

    bool Parser::Parse(char c) {
        if (status != OK) { return false; }
        ++charcount;
        if (c == '\n') {
            column = 0;
            ++line;
        } else {
            ++column;
        }
        if (JSON_parser_char(parser, c)) {
            status = OK;
            if (depth == 0) {
                if (JSON_parser_done(parser)) {
                    status = DONE;
                }
            }
            return true;
        } else if (status == OK) {
            status = ERROR;
        }
        return false;
    }

    unsigned Parser::Parse(const char *c, unsigned len) {
        unsigned i = 0;
        for (; i < len; ++i) {
            if (!Parse(c[i])) {
                break;
            }
        }
        return i;
    }

    int Parser::Callback(int type, const struct JSON_value_struct *value) {
        bool retval = false;
        switch(type) {
        case JSON_T_ARRAY_BEGIN:    
            retval = ArrayBegin();
            ++depth;
            break;
        case JSON_T_ARRAY_END:
            --depth;
            retval = ArrayEnd();
            break;
        case JSON_T_OBJECT_BEGIN:
            retval = ObjectBegin();
            ++depth;
            break;
        case JSON_T_OBJECT_END:
            --depth;
            retval = ObjectEnd();
            break;
        case JSON_T_INTEGER:
            retval = Integer(value->vu.integer_value);
            break;
        case JSON_T_FLOAT:
            retval = Float(value->vu.float_value);
            break;
        case JSON_T_NULL:
            retval = Null();
            break;
        case JSON_T_TRUE:
            retval = True();
            break;
        case JSON_T_FALSE:
            retval = False();
            break;
        case JSON_T_KEY:
            retval = Key(value->vu.str.value);
            break;
        case JSON_T_STRING:
            retval = String(value->vu.str.value);
            break;
        default:
            break;
        }
        return retval;
    }
}

std::istream &operator>>(std::istream &is, JSON::Parser &p) {
    while (p.Ok() && is.good() && p.Parse((char)is.get()));
    if (!p.Done()) is.unget();
    return is;
}

