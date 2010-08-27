//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */
#include "XMLToVariant.h"
#include "Assert.h"
#include "ParseBool.h"
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
    element["#content"] = Variant::ArrayType;
    element["#attr"] = Variant::ObjectType;
    for (AttributeList::const_iterator itr = attributes.begin(); itr != attributes.end(); ++itr) {
        element["#attr"][(string)itr->name] = (string)itr->value;
    }
    element["#type"] = element["#attr"]["type"];
    stack.push_back(element);
}

string TrimContent(Variant content) {
    std::ostringstream oss;
    Variant::ListIterator itr = content.ListBegin();
    Variant::ListIterator end = content.ListEnd();
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
    while (itr != end) {
        oss << itr->AsString();
        ++itr;
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
    if (empty) return string();
    return data;
}

void XMLToVariant::pimpl_t::on_end_element(const Glib::ustring& name) {
    if (state == ERROR) return;
    Variant element = stack.back();
    stack.pop_back();
    Variant parent = stack.back();
    if (element["#type"].IsNull()) {
        if (element["#data"].IsNull()) {
            element["#data"] = TrimContent(element["#content"]);
        }
    } else {
        string type = element["#type"].AsString();
        if ("object" == type) {
            /* nothing at this stage */
        } else if ("array" == type) {
            /* nothing at this stage */
        } else if ("number" == type) {
            std::istringstream iss(TrimContent(element["#content"]));
            long double val;
            iss >> val;
            element["#data"] = val;
        } else if ("bool" == type) {
            string data = TrimContent(element["#content"]);
            element["#data"] = ParseBool(data);
        } else if ("null" == type) {
            element["#data"] = Variant::NullType;
        } else /* string */ {
            element["#data"] = TrimContent(element["#content"]);
        }
    }
    bool is_object = false;
    if (parent["#type"].IsNull()) {
        is_object = true;
    } else {
        string type = parent["#type"].AsString();
        if ("array" == type) {
            parent["#data"].Append(element["#data"]);
        } else /* object */ {
            is_object = true;
        }
    }
    if (is_object) {
        if (parent["#data"][name].IsNull()) {
            parent["#data"][name] = element["#data"];
        } else if (parent["#data"][name].IsArray()) {
            parent["#data"][name].Append(element["#data"]);
        } else {
            Variant list = Variant::ArrayType;
            list.Append(parent["#data"][name]);
            list.Append(element["#data"]);
            parent["#data"][name] = list;
        }
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


void XMLToVariant::ParseChunk(const std::string &chunk) {
    ASSERT(pimpl->state == OK);
    pimpl->parse_chunk(chunk);
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

