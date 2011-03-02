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
#include "ThrowingAssert.h"
#include "ParseBool.h"
#include <sstream>

using std::tr1::shared_ptr;

static const Variant null;

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

Variant::~Variant() {
}

Variant &Variant::operator=(const Variant &v) {
    type = v.type;
    Assign(v);
    return *this;
}

void Variant::Assign(const Variant &v) {
    switch (type) {
    case NullType:
    case TrueType:
    case FalseType:
        stringval.clear();
        array.reset();
        object.reset();
        break;
    case NumberType:
        numvalue= v.numvalue;
        stringval.clear();
        array.reset();
        object.reset();
        break;
    case StringType:
        stringval = v.stringval;
        array.reset();
        object.reset();
        break;
    case ArrayType:
        array = v.array;
        stringval.clear();
        object.reset();
        break;
    case ObjectType:
        object = v.object;
        stringval.clear();
        array.reset();
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
        return ParseBool(stringval);
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
            oss.precision(50);
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
    if (NullType == type) {
        type = ArrayType;
        InitArray();
    }
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
    if (NullType == type) {
        type = ArrayType;
        InitArray();
    }
    ASSERT(type == ArrayType);
    return array->begin();
}

Variant::ConstListIterator Variant::ListBegin() const {
    ASSERT(type == ArrayType);
    return array->begin();
}

Variant::ListIterator Variant::ListEnd() {
    if (NullType == type) {
        type = ArrayType;
        InitArray();
    }
    ASSERT(type == ArrayType);
    return array->end();
}

Variant::ConstListIterator Variant::ListEnd() const {
    ASSERT(type == ArrayType);
    return array->end();
}


Variant::Map &Variant::AsObject() {
    if (NullType == type) {
        type = ObjectType;
        InitObject();
    }
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
    if (NullType == type) {
        type = ObjectType;
        InitObject();
    }
    ASSERT(type == ObjectType);
    return object->begin();
}

Variant::ConstMapIterator Variant::MapBegin() const {
    ASSERT(type == ObjectType);
    return object->begin();
}

Variant::MapIterator Variant::MapEnd() {
    if (NullType == type) {
        type = ObjectType;
        InitObject();
    }
    ASSERT(type == ObjectType);
    return object->end();
}

Variant::ConstMapIterator Variant::MapEnd() const {
    ASSERT(type == ObjectType);
    return object->end();
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

Variant::Type_t Variant::Contains(unsigned i) const {
    if (ArrayType == type) {
        if (array->size() <= i) {
            return NullType;
        }
        return array->at(i).GetType();
    }
    if (ObjectType == type) {
        std::ostringstream oss;
        oss << i;
        return Contains(oss.str());
    }
    ASSERT(false, "Not an indexible type");

}

Variant::Type_t Variant::Contains(const char *s) const {
    if (s == 0) {
        return Contains(unsigned(0));
    } else {
        return Contains(std::string(s));
    }
}

Variant::Type_t Variant::Contains(const std::string &s) const {
    if (ObjectType == type) {
        Map::iterator i = object->find(s);
        if (i == object->end()) {
            return NullType;
        }
        return i->second.GetType();
    }
    if (ArrayType == type) {
        std::istringstream iss(s);
        unsigned val = 0;
        iss >> val;
        ASSERT(!iss.fail(), "Failed to convert operand into index");
        return Contains(val);
    }
    ASSERT(false, "Invalid type");

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


