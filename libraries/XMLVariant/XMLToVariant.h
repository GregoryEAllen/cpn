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
 * This class provides a way of parsing
 * a simple XML document into a Variant. Note that
 * this operation is only reversable for very simple
 * documents that contain no attributes and whitespace
 * does not matter.
 *
 * Whitespace, commends, and most attributes are stripped.
 * All text inside an element that contains sub elements is
 * ignored. Empty elements are assumed to have the value 'true'.
 *
 * Currently all datatypes are assumed to be strings unless
 * there is a type attribute in which case the type attribute
 * causes this parser to try conver to that type. Otherwise user
 * must then convert them as necessary.
 *
 * Valid types are "object", "array", "string", "bool", "number"
 * and "null". An empty tag is considered to be an empty string.
 */
#ifndef XMLTOVARIANT_H
#define XMLTOVARIANT_H
#pragma once
#include "Variant.h"
#include <iosfwd>

/**
 * A parser that understand a simple XML langauge that can
 * be transformed into a Variant.
 * \see VariantToXML
 */
class XMLToVariant {
public:
    enum State_t {
        OK,
        DONE,
        ERROR
    };

    XMLToVariant();
    ~XMLToVariant();

    void ParseChunk(const std::string &chunk);
    void EndChunk();

    bool ParseFile(const std::string &f);
    bool ParseStream(std::istream &is);

    State_t State() const;
    std::string GetMessage() const;
    Variant Get();
    std::string RootName();

    bool Done() const;
    bool Ok() const;
    bool Error() const;
private:
    class pimpl_t;
    pimpl_t *pimpl;
};
#endif
