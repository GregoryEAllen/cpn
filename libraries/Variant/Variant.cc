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
 * Variant implementation
 */

#include "Variant.h"
#include "json.h"
#include "Assert.h"
#include "AutoBuffer.h"
#include <sstream>
#include <limits>
#include <stdlib.h>
#include <string.h>

using std::tr1::shared_ptr;

const Variant Variant::null;

Variant Variant::FromJSON(const std::string &jsonstring) {
    return FromJSON(jsonstring.c_str(), jsonstring.length());
}

Variant Variant::FromJSON(const char *jsonstring, unsigned len) {
    json_t *root = 0;
    try {
        // ehhh.... copy ick... needed to ensure the \0 at the end...
        AutoBuffer buffer(len+1);
        char *text = (char*)buffer.GetBuffer();
        memcpy(text, jsonstring, len);
        text[len] = '\0';
        json_error err = json_parse_document(&root, text);
        ASSERT(err == JSON_OK, "json_parse_document returned %d", err);
        Variant ret(root);
        json_free_value(&root);
        return ret;
    } catch (...) {
        if (root != 0) {
            json_free_value(&root);
        }
        throw;
    }
}

Variant Variant::FromJSON(const std::vector<char> &jsonstring) {
    return FromJSON(&jsonstring[0], jsonstring.size());
}

Variant::Variant(const json_t *root) {
    ASSERT(root, "Null argument");
    char *str = 0;
    try {
        switch (root->type) {
        case JSON_STRING:
            type = StringType;
            str = json_unescape(root->text);
            stringval = str;
            free(str);
            str = 0;
            break;
        case JSON_NUMBER:
            type = NumberType;
            {
                std::istringstream iss(root->text);
                iss >> numvalue;
                ASSERT(!iss.fail(), "Number conversion failed");
            }
            break;
        case JSON_OBJECT:
            type = ObjectType;
            InitObject();
            {
                const json_t *i = root->child; 
                while (i) {
                    // mjson stores the pair as a string value for the key then it has
                    // a single child which contains the value
                    str = json_unescape(i->text);
                    object->insert(std::make_pair(std::string(str), Variant(i->child)));
                    free(str);
                    str = 0;
                    if (i == root->child_end) {
                        break;
                    }
                    i = i->next;
                }
            }
            break;
        case JSON_ARRAY:
            type = ArrayType;
            InitArray();
            {
                const json_t *i = root->child; 
                while (i) {
                    array->push_back(Variant(i));
                    if (i == root->child_end) {
                        break;
                    }
                    i = i->next;
                }
            }
            break;
        case JSON_TRUE:
            type = TrueType;
            break;
        case JSON_FALSE:
            type = FalseType;
            break;
        case JSON_NULL:
            type = NullType;
            break;
        default:
            ASSERT(false, "Unexpected type %d", root->type);
        }
    } catch (...) {
        free(str);
        str = 0;
        throw;
    }
}

Variant::Variant() : type(NullType) {}

Variant::Variant(Type_t type_) : type(type_) {
    switch (type) {
    case NullType:
    case TrueType:
    case FalseType:
        break;
    case NumberType:
        numvalue = 0;
    case StringType:
        break;
    case ArrayType:
        InitArray();
        break;
    case ObjectType:
        InitObject();
        break;
    default:
        ASSERT(false, "Unknown type %d", type);
    }
}

Variant::Variant(bool v) {
    if (v) {
        type = TrueType;
    } else {
        type = FalseType;
    }
}

Variant::Variant(int v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(unsigned v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(long v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(unsigned long v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(long long v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(unsigned long long v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(double v) : type(NumberType) {
    numvalue = v;
}

Variant::Variant(long double v) : type(NumberType) {
    numvalue = v;
}


Variant::Variant(const std::string &v) : type(StringType), stringval(v) {}

Variant::Variant(const char *v) : type(StringType) {
    if (v == 0) {
        type = NumberType;
        numvalue = 0;
    } else {
        stringval = v;
    }
}

Variant::Variant(const Variant &v) : type(v.type) {
    Assign(v);
}

Variant &Variant::operator=(const Variant &v) {
    type = v.type;
    Assign(v);
    return *this;
}

void Variant::Assign(const Variant &v) {
    stringval.clear();
    array.reset();
    object.reset();
    switch (type) {
    case NullType:
    case TrueType:
    case FalseType:
        break;
    case NumberType:
        numvalue= v.numvalue;
        break;
    case StringType:
        stringval = v.stringval;
        break;
    case ArrayType:
        array = v.array;
        break;
    case ObjectType:
        object = v.object;
        break;
    default:
        ASSERT(false, "Unknown type %d", type);
    }
}

void Variant::InitArray() {
    ASSERT(type == ArrayType);
    array = shared_ptr<std::vector<Variant> >(new std::vector<Variant>());
}

void Variant::InitObject() {
    ASSERT(type == ObjectType);
    object = shared_ptr<std::map<std::string, Variant> >(new std::map<std::string, Variant>());
}

void Variant::Swap(Variant& other) {
    Variant temp = other;
    other.type = type;
    other.Assign(*this);
    type = temp.type;
    Assign(temp);
}

bool Variant::AsBool() const {
    switch (type) {
    case NumberType:
        return numvalue != 0;
    case StringType:
        {
            std::istringstream iss(stringval);
            bool val = false;
            iss >> val;
            ASSERT(!iss.fail(), "Connot convert \"%s\" to bool", stringval.c_str());
            return val;
        }
    case TrueType:
        return true;
    case FalseType:
    case NullType:
        return false;
    case ArrayType:
    case ObjectType:
    default:
        ASSERT(false, "Cannot convert to bool");
    }
}

long double Variant::AsLongDouble() const {
    switch (type) {
    case NumberType:
        return numvalue;
    case StringType:
        {
            std::istringstream iss(stringval);
            long double val = 0;
            iss >> val;
            ASSERT(!iss.fail(), "Connot convert \"%s\" to number", stringval.c_str());
            return val;
        }
    case TrueType:
        return 1;
    case FalseType:
    case NullType:
        return 0;
    case ArrayType:
    case ObjectType:
    default:
        ASSERT(false, "Connot convert to number");
    }
}

const std::string &Variant::AsString() const {
    if (!stringval.empty()) {
        return stringval;
    }
    switch (type) {
    case NumberType:
        {
            std::ostringstream oss;
            oss << numvalue;
            ASSERT(!oss.fail(), "Failed to convert number to string.");
            stringval = oss.str();
            return stringval;
        }
    case StringType:
        return stringval;
    case TrueType:
        stringval = "true";
        return stringval;
    case FalseType:
        stringval = "false";
        return stringval;
    case NullType:
        stringval = "null";
        return stringval;
    case ArrayType:
    case ObjectType:
    default:
        ASSERT(false, "Connot convert to string.");
    }

}

Variant::List &Variant::AsArray() {
    switch (type) {
    case ArrayType:
        return *array;
    default:
        ASSERT(false, "Not an array");
    }
}

const Variant::List &Variant::AsArray() const {
    switch (type) {
    case ArrayType:
        return *array;
    default:
        ASSERT(false, "Not an array");
    }
}

Variant::ListIterator Variant::ListBegin() {
    ASSERT(type == ArrayType);
    return array->begin();
}

Variant::ConstListIterator Variant::ListBegin() const {
    ASSERT(type == ArrayType);
    return array->begin();
}

Variant::ListIterator Variant::ListEnd() {
    ASSERT(type == ArrayType);
    return array->end();
}

Variant::ConstListIterator Variant::ListEnd() const {
    ASSERT(type == ArrayType);
    return array->end();
}


Variant::Map &Variant::AsObject() {
    switch (type) {
    case ObjectType:
        return *object;
    default:
        ASSERT(false, "Not an object");
    }
}

const Variant::Map &Variant::AsObject() const {
    switch (type) {
    case ObjectType:
        return *object;
    default:
        ASSERT(false, "Not an object");
    }
}

Variant::MapIterator Variant::MapBegin() {
    ASSERT(type == ObjectType);
    return object->begin();
}

Variant::ConstMapIterator Variant::MapBegin() const {
    ASSERT(type == ObjectType);
    return object->begin();
}

Variant::MapIterator Variant::MapEnd() {
    ASSERT(type == ObjectType);
    return object->end();
}

Variant::ConstMapIterator Variant::MapEnd() const {
    ASSERT(type == ObjectType);
    return object->end();
}

json_t *Variant::BuildJSONTree() const {
    json_t *ret = 0;
    json_t *value = 0;
    char *str = 0;
    try {
        switch (type) {
        case ObjectType:
            ret = json_new_object();
            for (Map::iterator i = object->begin(); i != object->end(); ++i) {
                value = i->second.BuildJSONTree();
                str = json_escape(i->first.c_str());
                json_error err = json_insert_pair_into_object(ret, str, value);
                free(str);
                str = 0;
                value = 0;
                ASSERT(err == JSON_OK, "json_insert_pair_into_object returned %d", err);
            }
            break;
        case ArrayType:
            ret = json_new_array();
            for (List::iterator i = array->begin(); i != array->end(); ++i) {
                value = i->BuildJSONTree();
                json_error err = json_insert_child(ret, value);
                value = 0;
                ASSERT(err == JSON_OK, "json_insert_child returned %d", err);
            }
            break;
        case StringType:
            {
                str = json_escape(AsString().c_str());
                ret = json_new_string(str);
                free(str);
                str = 0;
            }
            break;
        case NumberType:
            ret = json_new_number(AsString().c_str());
            break;
        case FalseType:
            ret = json_new_false();
            break;
        case TrueType:
            ret = json_new_true();
            break;
        case NullType:
            ret = json_new_null();
            break;
        default:
            ASSERT(false, "Unknown type %d", type);
        }
        return ret;
    } catch (...) {
        if (ret != 0) {
            json_free_value(&ret);
        }
        if (value != 0) {
            json_free_value(&value);
        }
        free(str);
        str = 0;
        throw;
    }
}

std::string Variant::AsJSON() const {
    char *text = 0;
    json_t *root = 0;
    try {
        root = BuildJSONTree();
        json_error err = json_tree_to_string(root, &text);
        ASSERT(err == JSON_OK, "json_tree_to_string returned %d", err);
        json_free_value(&root);
        std::string ret = text;
        free(text);
        text = 0;
        return ret;
    } catch (...) {
        free(text);
        if (root != 0) {
            json_free_value(&root);
        }
        throw;
    }
}

int Variant::AsJSON(char *target, unsigned maxlen) const {
    char *text = 0;
    json_t *root = 0;
    try {
        root = BuildJSONTree();
        json_error err = json_tree_to_string(root, &text);
        ASSERT(err == JSON_OK, "json_tree_to_string returned %d", err);
        ASSERT(text);
        unsigned len = strlen(text);
        strncpy(target, text, maxlen);
        free(text);
        text = 0;
        json_free_value(&root);
        return len;
    } catch (...) {
        free(text);
        if (root != 0) {
            json_free_value(&root);
        }
        throw;
    }
}

void Variant::AsJSON(std::vector<char> &target) const {
    char *text = 0;
    json_t *root = 0;
    try {
        root = BuildJSONTree();
        json_error err = json_tree_to_string(root, &text);
        ASSERT(err == JSON_OK, "json_tree_to_string returned %d", err);
        ASSERT(text);
        unsigned len = strlen(text);
        target.assign(text, text + len);
        free(text);
        text = 0;
        json_free_value(&root);
    } catch (...) {
        free(text);
        text = 0;
        if (root != 0) {
            json_free_value(&root);
        }
        throw;
    }
}

unsigned Variant::Size() const {
    switch (type) {
    case ArrayType:
        return array->size();
    case ObjectType:
        return object->size();
    case StringType:
        return stringval.size();
    case FalseType:
    case TrueType:
    case NullType:
    default:
        ASSERT(false, "Type does not have the concept of size.");
    }
}

bool Variant::Empty() const {
    switch (type) {
    case ArrayType:
        return array->empty();
    case ObjectType:
        return object->empty();
    case StringType:
        return stringval.empty();
    case NullType:
    case FalseType:
    case TrueType:
    default:
        ASSERT(false, "Type does not have the concept of empty.");
    }
}

void Variant::Clear() {
    switch (type) {
    case ArrayType:
        return array->clear();
    case ObjectType:
        return object->clear();
    case StringType:
        return stringval.clear();
    default:
        type = NullType;
        break;
    }
}

Variant Variant::Copy() const {
    Variant ret(type);
    switch (type) {
    case ArrayType:
        for (ConstListIterator itr = ListBegin(); itr != ListEnd(); ++itr) {
            ret.Append(itr->Copy());
        }
        break;
    case ObjectType:
        for (ConstMapIterator itr = MapBegin(); itr != MapEnd(); ++itr) {
            ret.At(itr->first) = itr->second.Copy();
        }
        break;
    case StringType:
        ret.stringval = stringval;
        break;
    case NumberType:
        ret.numvalue = numvalue;
        break;
    case FalseType:
    case TrueType:
    case NullType:
        break;
    }
    return ret;
}

Variant &Variant::Append(const Variant &value) {
    if (NullType == type) {
        type = ArrayType;
        InitArray();
    }
    ASSERT(ArrayType == type);
    array->push_back(value);
    return *this;
}

Variant &Variant::At(unsigned i) {
    if (NullType == type) {
        type = ArrayType;
        InitArray();
    }
    if (ArrayType == type) {
        if (array->size() <= i) {
            array->resize(i + 1);
        }
        return array->at(i);
    }
    if (ObjectType == type) {
        std::ostringstream oss;
        oss << i;
        return object->insert(std::make_pair(oss.str(), null)).first->second;
    }
    ASSERT(false, "Not an indexible type");
}

const Variant &Variant::At(unsigned i) const {
    if (ArrayType == type) {
        return array->at(i);
    }
    if (ObjectType == type) {
        std::ostringstream oss;
        oss << i;
        return object->find(oss.str())->second;
    }
    ASSERT(false, "Not an indexible type");
}

Variant &Variant::At(const char *s) {
    if (0 == s) {
        return At(unsigned(0));
    }
    if (NullType == type) {
        type = ObjectType;
        InitObject();
    }
    if (ObjectType == type) {
        return object->insert(std::make_pair(std::string(s), null)).first->second;
    }
    if (ArrayType == type) {
        std::istringstream iss(s);
        unsigned val = 0;
        iss >> val;
        ASSERT(!iss.fail(), "Failed to convert operand into index");
        return At(val);
    }
    ASSERT(false, "Invalid type");
}

const Variant &Variant::At(const char *s) const {
    if (0 == s) {
        return At(unsigned(0));
    }
    if (ObjectType == type) {
        Map::iterator entry = object->find(s);
        if (entry != object->end()) {
            return entry->second;
        } else {
            return null;
        }
    }
    if (ArrayType == type) {
        std::istringstream iss(s);
        unsigned val = 0;
        iss >> val;
        ASSERT(!iss.fail(), "Failed to convert operand into index");
        return At(val);
    }
    ASSERT(false, "Invalid type");

}

Variant &Variant::At(const std::string &s) {
    if (NullType == type) {
        type = ObjectType;
        InitObject();
    }
    if (ObjectType == type) {
        return object->insert(std::make_pair(s, null)).first->second;
    }
    if (ArrayType == type) {
        std::istringstream iss(s);
        unsigned val = 0;
        iss >> val;
        ASSERT(!iss.fail(), "Failed to convert operand into index");
        return At(val);
    }
    ASSERT(false, "Invalid type");
}

const Variant &Variant::At(const std::string &s) const {
    if (ObjectType == type) {
        Map::iterator val = object->find(s);
        if (object->end() == val) {
            return null;
        }
        return val->second;
    }
    if (ArrayType == type) {
        std::istringstream iss(s);
        unsigned val = 0;
        iss >> val;
        ASSERT(!iss.fail(), "Failed to convert operand into index");
        return At(val);
    }
    ASSERT(false, "Invalid type");
}

int Variant::Compare(const Variant &other) const {
    switch (other.GetType()) {
    case ArrayType:
        if (IsArray()) {
            return AsArray() == other.AsArray();
        } else {
            return 1;
        }
    case ObjectType:
        if (IsObject()) {
            return AsObject() == other.AsObject();
        } else {
            return 1;
        }
    case StringType:
        return CompareString(other);
    case NumberType:
        return CompareNumber(other);
    case NullType:
        if (other.IsNull()) {
            return 0;
        } else {
            return 1;
        }
    case FalseType:
        if (other.IsFalse()) {
            return 0;
        } else { return 1; }
    case TrueType:
        if (other.IsTrue()) {
            return 0;
        } else { return 1; }
    default:
        ASSERT(false, "Invalid type %d", type);
    }
}

int Variant::CompareString(const Variant &other) const {
    // We assume other is of type string
    switch (type) {
    case ArrayType:
        return 1;
    case ObjectType:
        return 1;
    case StringType:
        return stringval.compare(other.stringval);
    case NumberType:
        {
            long double us = numvalue;
            long double o = other.AsLongDouble();
            // TODO check if AsLongDouble failed
            // if it did do string compare
            if (us < o) { return -1; }
            else if (us > o) { return 1; }
            else { return 0; }
        }
    case NullType:
        return 1;
    case FalseType:
        if (other.AsBool()) {
            return 1;
        } else {
            return 0;
        }
    case TrueType:
        if (other.AsBool()) {
            return 0;
        } else {
            return 1;
        }
    default:
        ASSERT(false, "Invalid type %d", type);
    }
}

int Variant::CompareNumber(const Variant &other) const {
    // Assume other is of type number
    switch (type) {
    case ArrayType:
        return 1;
    case ObjectType:
        return 1;
    case StringType:
        // TODO attempt to convert other into a number
        // if fail then do string compare
        return AsString().compare(other.AsString());
    case NumberType:
        {
            long double us = numvalue;
            long double o = other.numvalue;
            if (us < o) { return -1; }
            else if (us > o) { return 1; }
            else { return 0; }
        }
    case NullType:
        return 1;
    case FalseType:
        return 0;
    case TrueType:
        return 0;
    default:
        ASSERT(false, "Invalid type %d", type);
    }
}


