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
#include "VariantToJSON.h"
#include <sstream>
#include <stdio.h>

const char OBJ_SEP[] = ": ";
const char VAL_SEP[] = ", ";

std::string VariantToJSON(const Variant &v, bool pretty) {
    std::ostringstream oss;
    VariantToJSON(oss, v, pretty);
    return oss.str();
}

static void InsertString(std::ostream &os, const std::string str) {
    os << "\"";
    const char *text = str.c_str();
    while (*text != '\0') {
        if (*text == '\\') {
            os << "\\\\";
        } else if (*text == '\"') {
            os << "\\\"";
        } else if (*text == '/') {
            os << "\\/";
        } else if (*text == '\b') {
            os << "\\b";
        } else if (*text == '\f') {
            os << "\\f";
        } else if (*text == '\n') {
            os << "\\n";
        } else if (*text == '\r') {
            os << "\\r";
        } else if (*text == '\t') {
            os << "\\t";
        } else if (*text < 0)   {
            os << *text;
        } else if (*text < 0x20) {
            char buffer[7];
            snprintf(buffer, sizeof(buffer), "\\u%4.4x", *text);
            os << buffer;
        } else {
            os << *text;
        }
        ++text;
    }
    os << "\"";
}

static void InsertIndent(std::ostream &os, unsigned level) {
#if 0
    static const char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
    static const unsigned len = sizeof(tabs) - 1;
    if (level < len) {
        os << "\n" << &tabs[len - level];
    } else {
        os << "\n";
        for (unsigned j = 0; j < level; ++j) {
            os << "\t";
        }
    }
#else
    // Must be a multiple of tabwidth
    static const char spaces[] = "                                ";
    static const unsigned len = sizeof(spaces) - 1;
    static const unsigned tabwidth = 2;
    static const unsigned tabs = len/tabwidth;
    os << "\n";
    while (level > 0) {
        if (level < tabs) {
            os << &spaces[len - level*tabwidth];
            return;
        } else {
            os << spaces;
            level -= tabs;
        }
    }
#endif
}

static std::ostream &VariantToJSON(std::ostream &os, const Variant &v, bool pretty, unsigned level) {
    switch (v.GetType()) {
    case Variant::NullType:
    case Variant::TrueType:
    case Variant::FalseType:
    case Variant::NumberType:
        os << v.AsString();
        break;
    case Variant::StringType:
        InsertString(os, v.AsString());
        break;
    case Variant::ArrayType:
        os << "[";
        for (Variant::ConstListIterator i = v.ListBegin(); i != v.ListEnd(); ++i) {
            if (i != v.ListBegin()) { os << VAL_SEP; }
            if (pretty) { InsertIndent(os, level); }
            VariantToJSON(os, *i, pretty, level + 1);
        }
        if (pretty) { InsertIndent(os, level - 1); }
        os << "]";
        break;
    case Variant::ObjectType:
        os << "{";
        for (Variant::ConstMapIterator i = v.MapBegin(); i != v.MapEnd(); ++i) {
            if (i != v.MapBegin()) { os << VAL_SEP; }
            if (pretty) { InsertIndent(os, level); }
            InsertString(os, i->first);
            os << OBJ_SEP;
            VariantToJSON(os, i->second, pretty, level + 1);
        }
        if (pretty) { InsertIndent(os, level - 1); }
        os << "}";
        break;
    default:
        break;
    }
    return os;
}

std::ostream &VariantToJSON(std::ostream &os, const Variant &v, bool pretty) {
    return VariantToJSON(os, v, pretty, 1);
}

