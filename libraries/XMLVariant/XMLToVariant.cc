
#include "XMLToVariant.h"
#include "Assert.h"
#include <libxml++/libxml++.h>
#include <deque>
#include <sstream>
#include <cctype>
#include <iostream>

#if 0
#define LOG(arg) std::cerr << arg << std::endl
#else
#define LOG(arg)
#endif

using std::deque;
using std::string;
using std::isspace;

class XMLToVariant::pimpl_t : public xmlpp::SaxParser {
public:
    virtual void on_start_document();
    virtual void on_end_document();
    virtual void on_start_element(const Glib::ustring& name, const AttributeList& attributes);
    virtual void on_end_element(const Glib::ustring& name);
    virtual void on_characters(const Glib::ustring& characters);
    //virtual void on_comment(const Glib::ustring& text);
    virtual void on_warning(const Glib::ustring& text) {
        LOG("WARNING: " << text);
    }
    virtual void on_error(const Glib::ustring& text) {
        LOG("ERROR: " << text);
        Error(text);
    }
    virtual void on_fatal_error(const Glib::ustring& text) {
        LOG("FATAL ERROR: " << text);
        Error(text);
        xmlpp::SaxParser::on_fatal_error(text);
    }
    virtual void on_cdata_block(const Glib::ustring& text);

    void Error(const std::string str) {
        if (state != ERROR) {
            state = ERROR;
            message = str;
        }
    }
    Variant result;
    std::string rootname;
    std::string message;
    deque<Variant> stack;
    State_t state;
};

void XMLToVariant::pimpl_t::on_start_document() {
    if (state == ERROR) return;
    Variant root = Variant::ObjectType;
    root["#data"] = Variant::ObjectType;
    root["#content"] = Variant::ArrayType;
    stack.push_back(root);
}

void XMLToVariant::pimpl_t::on_end_document() {
    if (state == ERROR) return;
    if (stack.size() > 1) {
        state = ERROR;
        Error("stack size to large!?");
    } else {
        state = DONE;
        Variant res = stack.back().At("#data");
        stack.pop_back();
        rootname = res.MapBegin()->first;
        result = res.MapBegin()->second;
    }
}

void XMLToVariant::pimpl_t::on_start_element(const Glib::ustring& name,
        const AttributeList& attributes) {
    if (state == ERROR) return;
    Variant element = Variant::ObjectType;
    element["#name"] = (std::string)name;
    element["#data"] = Variant::ObjectType;
    element["#content"] = Variant::ArrayType;
    stack.push_back(element);
}

void AddData(Variant parent, Variant data, const string &name) {
    Variant store = parent["#data"];
    if (store[name].IsNull()) {
        store[name] = data;
    } else if (store[name].IsArray()) {
        store[name].Append(data);
    } else {
        Variant list = Variant::ArrayType;
        list.Append(store[name]);
        list.Append(data);
        store[name] = list;
    }
}

void XMLToVariant::pimpl_t::on_end_element(const Glib::ustring& name) {
    if (state == ERROR) return;
    Variant element = stack.back();
    stack.pop_back();
    Variant parent = stack.back();
    if (!element["#data"].Empty()) {
        AddData(parent, element["#data"], element["#name"].AsString());
    } else if (!element["#content"].Empty()) {
        std::ostringstream oss;
        Variant::ListIterator itr = element["#content"].ListBegin();
        Variant::ListIterator end = element["#content"].ListEnd();
        bool loop = true;
        while (itr != end && loop) {
            string text = itr->AsString();
            for (unsigned i = 0; i < text.size(); ++i) {
                if (!std::isspace(text[i])) {
                    loop = false;
                    oss << text.substr(i);
                    break;
                }
            }
            ++itr;
        }
        while (itr != end && loop) {
            oss << itr->AsString();
        }
        string data = oss.str();
        bool empty = true;
        for (unsigned i = data.size(); i > 0; --i) {
            if (!std::isspace(data[i - 1])) {
                data = data.substr(0, i);
                empty = false;
                break;
            }
        }
        if (empty) {
            AddData(parent, true, element["#name"].AsString());
        } else {
            // Would be nice to try to convert to actual datatypes rather than strings
            AddData(parent, data, element["#name"].AsString());
        }
    } else {
        AddData(parent, true, element["#name"].AsString());
    }
}

void XMLToVariant::pimpl_t::on_characters(const Glib::ustring& characters) {
    if (state == ERROR) return;
    Variant element = stack.back();
    element["#content"].Append((std::string)characters);
}

void XMLToVariant::pimpl_t::on_cdata_block(const Glib::ustring& text) {
    if (state == ERROR) return;
    Variant element = stack.back();
    element["#content"].Append((std::string)text);
}

XMLToVariant::XMLToVariant() {
    pimpl = new pimpl_t;
}

XMLToVariant::~XMLToVariant() {
    delete pimpl;
    pimpl = 0;
}


void XMLToVariant::ParseChunk(const void *ptr, unsigned len) {
    ASSERT(pimpl->state == OK);
    pimpl->parse_chunk_raw((const unsigned char*)ptr, len);
}

void XMLToVariant::EndChunk() {
    pimpl->finish_chunk_parsing();
}

bool XMLToVariant::ParseFile(const std::string &f) {
    pimpl->parse_file(f);
    return pimpl->state == DONE;
}

bool XMLToVariant::ParseStream(std::istream &is) {
    pimpl->parse_stream(is);
    return pimpl->state == DONE;
}

XMLToVariant::State_t XMLToVariant::State() const {
    return pimpl->state;
}

std::string XMLToVariant::GetMessage() const {
    return pimpl->message;
}

Variant XMLToVariant::Get() {
    ASSERT(pimpl->state == DONE);
    return pimpl->result;
}

std::string XMLToVariant::RootName() {
    ASSERT(pimpl->state == DONE);
    return pimpl->rootname;
}

bool XMLToVariant::Done() const {
    return pimpl->state == DONE;
}

bool XMLToVariant::Ok() const {
    return pimpl->state == OK;
}

bool XMLToVariant::Error() const {
    return pimpl->state == ERROR;
}

