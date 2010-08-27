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
#ifndef JSONTOVARIANT_H
#define JSONTOVARIANT_H
#pragma once

#include "JSONParser.h"
#include "Variant.h"
#include <stack>

/**
 * An implementation of the JSON::Parser that
 * constructs a Variant
 */
class JSONToVariant : public JSON::Parser {
public:
    JSONToVariant();
    ~JSONToVariant();
    /**
     * \return the Variant constructed.
     */
    Variant Get() const { return stack.top(); }
    void Reset();
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

    bool AddValue(Variant val);
    bool InObject();
    bool InArray();

    std::stack<std::string> keystack;
    typedef std::stack<Variant> Stack;
    Stack stack;
};

/**
 * Construct a Variant form a JSON string.
 * \param json the string
 * \return the Variant
 */
Variant VariantFromJSON(const std::string &json);
/**
 * Construct a Variant from a std::istream
 * \param is the stream
 * \return The Variant
 */
Variant VariantFromJSON(std::istream &is);

#endif
