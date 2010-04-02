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
 *
 * \brief This is a variant type inspired by types from several different
 * scripting languages.
 *
 * \author John Bridgman
 *
 * Arrays and Objects are reference counted using std::tr1::shared_ptr.  If one
 * wants to copy use the Copy member function.
 *
 * Numbers are all internally represented as a long double which should be
 * able to represent all integer types exactly (i.e. +-2^100 something)
 * that is to say, way overkill
 *
 * Note that this will let you create a back reference but unless the user
 * explicitely breaks it, it will result in a memory leak.  Also, the back
 * reference will cause the ToJSON function to cause a stack overflow.
 *
 * This uses the mjson library (see NOTES in mjson folder) to provide
 * json serialization and deserialization.
 *
 * This class currently uses the ASSERT macro for all error checking.
 * It might make more sense to throw different exceptions on some errors
 * like a bad_cast or something.
 */

#ifndef VARIANT_H
#define VARIANT_H
#pragma once
#include <string>
#include <tr1/memory>
#include <vector>
#include <map>

struct json_value;

class Variant {
public:
    typedef std::vector<Variant> List;
    typedef std::map<std::string, Variant> Map;

    typedef Map::iterator MapIterator;
    typedef Map::const_iterator ConstMapIterator;
    typedef List::iterator ListIterator;
    typedef List::const_iterator ConstListIterator;

    enum Type_t {
        NullType,
        TrueType,
        FalseType,
        NumberType,
        StringType,
        ArrayType,
        ObjectType
    };

    static const Variant null;

    static Variant FromJSON(const std::string &jsonstring);
    static Variant FromJSON(const char *jsonstring, unsigned len);
    static Variant FromJSON(const std::vector<char> &jsonstring);

    Variant();
    Variant(Type_t type_);
    Variant(bool v);

    Variant(int v);
    Variant(unsigned v);
    Variant(long v);
    Variant(unsigned long v);
    Variant(long long v);
    Variant(unsigned long long v);
    Variant(double v);
    Variant(long double v);

    Variant(const std::string &v);
    Variant(const char *v);
    Variant(const Variant &v);
    // Build a Variant from a json_value
    Variant(const json_value *root);

    Variant &operator=(const Variant &v);

    bool operator!() const { return IsNull() || IsFalse(); }
    operator bool() const { return !IsNull() && !IsFalse(); }

    void Swap(Variant &other);

    bool AsBool() const;
    long double AsLongDouble() const;
    double AsDouble() const { return static_cast<double>(AsLongDouble()); }
    unsigned AsUnsigned() const { return static_cast<unsigned>(AsLongDouble()); }
    int AsInt() const { return static_cast<int>(AsLongDouble()); }

    template<typename T> T AsNumber() const { return static_cast<T>(AsLongDouble()); }

    const std::string &AsString() const;
    List &AsArray();
    const List &AsArray() const;

    ListIterator ListBegin();
    ConstListIterator ListBegin() const;
    ListIterator ListEnd();
    ConstListIterator ListEnd() const;

    Map &AsObject();
    const Map &AsObject() const;

    MapIterator MapBegin();
    ConstMapIterator MapBegin() const;
    MapIterator MapEnd();
    ConstMapIterator MapEnd() const;

    std::string AsJSON() const;
    int AsJSON(char *target, unsigned maxlen) const;
    void AsJSON(std::vector<char> &target) const;

    Type_t GetType() const { return type; }

    bool IsNull() const { return NullType == type; }
    bool IsBool() const { return TrueType == type || FalseType == type; }
    bool IsTrue() const { return TrueType == type; }
    bool IsFalse() const { return FalseType == type; }
    bool IsNumber() const { return NumberType == type; }
    bool IsString() const { return StringType == type; }
    bool IsArray() const { return ArrayType == type; }
    bool IsObject() const { return ObjectType == type; }

    unsigned Size() const;
    bool Empty() const;
    void Clear();
    Variant Copy() const;

    // Only valid when of type ArrayType
    // Returns *this, so you can do
    // Variant().Append(v1).Append(v2)...
    Variant &Append(const Variant &value);

    Variant &At(unsigned i);
    const Variant &At(unsigned i) const;

    Variant &At(const char *s);
    const Variant &At(const char *s) const;

    Variant &At(const std::string &s);
    const Variant &At(const std::string &s) const;

    Variant &operator[](int i) { return At(i); }
    const Variant &operator[](int i) const { return At(i); }

    Variant &operator[](unsigned i) { return At(i); }
    const Variant &operator[](unsigned i) const { return At(i); }

    Variant &operator[](const char *s) { return At(s); }
    const Variant &operator[](const char *s) const { return At(s); }

    Variant &operator[](const std::string &s) { return At(s); }
    const Variant &operator[](const std::string &s) const { return At(s); }

    // Returns -1 if we are less than other
    // 0 if we are equal
    // 1 if we are greater than
    // Note that null, true, false, array and object types
    // will always return 1 when different
    // This compare is not completely consistent
    // i.e. it can be such that two Variants are
    // both < and > (ex object and array types)
    int Compare(const Variant &other) const;
private:
    void Assign(const Variant &v);
    void InitArray();
    void InitObject();
    json_value *BuildJSONTree() const;

    int CompareString(const Variant &other) const;
    int CompareNumber(const Variant &other) const;

    Type_t type;
    // long double is 128 bits on most systems which can represent most other
    // number types exactly
    long double numvalue;
    mutable std::string stringval;
    std::tr1::shared_ptr<List> array;
    std::tr1::shared_ptr<Map> object;

};

inline bool operator==(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) == 0; }
inline bool operator!=(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) != 0; }
inline bool operator<(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) < 0; }
inline bool operator>(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) > 0; }
inline bool operator<=(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) <= 0; }
inline bool operator>=(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) >= 0; }

#endif
