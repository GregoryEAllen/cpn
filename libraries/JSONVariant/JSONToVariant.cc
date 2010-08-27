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
#include "JSONToVariant.h"
#include "Assert.h"

JSONToVariant::JSONToVariant() {
}

JSONToVariant::~JSONToVariant() {
}

void JSONToVariant::Reset() {
    while (!keystack.empty()) { keystack.pop(); }
    while (!stack.empty()) { stack.pop(); }
    JSON::Parser::Reset();
}

bool JSONToVariant::ArrayBegin() {
    stack.push(Variant(Variant::ArrayType));
    return true;
}

bool JSONToVariant::ArrayEnd() {
    Variant v = stack.top();
    if (!v.IsArray()) return false;
    stack.pop();
    return AddValue(v);
}

bool JSONToVariant::ObjectBegin() {
    stack.push(Variant(Variant::ObjectType));
    return true;
}

bool JSONToVariant::ObjectEnd() {
    Variant v = stack.top();
    if (!v.IsObject()) return false;
    stack.pop();
    return AddValue(v);
}

bool JSONToVariant::Integer(int64_t value) {
    Variant val = value;
    return AddValue(val);
}

bool JSONToVariant::Float(double value) {
    Variant val = value;
    return AddValue(val);
}

bool JSONToVariant::String(const std::string &str) {
    Variant val = str;
    return AddValue(val);
}

bool JSONToVariant::Null() {
    return AddValue(Variant(Variant::NullType));
}

bool JSONToVariant::True() {
    return AddValue(Variant(Variant::TrueType));
}

bool JSONToVariant::False() {
    return AddValue(Variant(Variant::FalseType));
}

bool JSONToVariant::Key(const std::string &str) {
    if (InObject()) {
        keystack.push(str);
        return true;
    }
    return false;
}

bool JSONToVariant::AddValue(Variant val) {
    if (InObject()) {
        if (keystack.empty()) return false;
        if (stack.top().Contains(keystack.top()) != Variant::NullType) return false;
        stack.top().At(keystack.top()) = val;
        keystack.pop();
    } else if (InArray()) {
        stack.top().Append(val);
    } else {
        stack.push(val);
    }
    return true;
}

bool JSONToVariant::InObject() {
    if (stack.empty()) return false;
    return stack.top().IsObject();
}

bool JSONToVariant::InArray() {
    if (stack.empty()) return false;
    return stack.top().IsArray();
}

Variant VariantFromJSON(const std::string &json) {
    JSONToVariant parser;
    parser.Parse(json);
    ASSERT(parser.Done(), "Error parsing JSON line %u column %u",
            parser.GetLine(), parser.GetColumn());
    return parser.Get();
}
Variant VariantFromJSON(std::istream &is) {
    JSONToVariant parser;
    is >> parser;
    ASSERT(parser.Done(), "Error parsing JSON line %u column %u",
            parser.GetLine(), parser.GetColumn());
    return parser.Get();
}

