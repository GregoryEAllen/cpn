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
#ifndef VARIANTTOJSON_H
#define VARIANTTOJSON_H
#pragma once
#include <iosfwd>
#include "Variant.h"

/**
 * Operator class to be used in output streams to specify JSON formatting.
 */
class PrettyJSON {
public:
    explicit PrettyJSON(const Variant &v, bool p = true) : value(v), pretty(p) {}
    const Variant &value;
    const bool pretty;
};

/**
 * Take a Variant and transform it into the equivalent JSON string.
 * \param v the Variant
 * \param pretty to pretty print or not (default: false)
 * \return the string
 */
std::string VariantToJSON(const Variant &v, bool pretty = false);

/**
 * Output a JSON string to the given output stream from the Variant.
 * \param os the output stream to output on
 * \param v The variant to construct the JSON from.
 * \param pretty to pretty print or not.
 * \return the os stream.
 */
std::ostream &VariantToJSON(std::ostream &os, const Variant &v, bool pretty);

inline std::ostream &operator<<(std::ostream &os, const Variant &v) {
    return VariantToJSON(os, v, false);
}

inline std::ostream &operator<<(std::ostream &os, const PrettyJSON &v) {
    return VariantToJSON(os, v.value, v.pretty);
}

#endif
