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
 * \brief Defintions of and helper functions for queue datatypes.
 * \author John Bridgman
 */

#ifndef CPN_QUEUEDATATYPES_H
#define CPN_QUEUEDATATYPES_H
#pragma once

#include "common.h"
#include <stdint.h>
#include <typeinfo>
#include <cstring>

namespace CPN {

    template<class type>
    const char* TypeName() {
        return typeid(type).name();
    }

    template<class type>
    bool CompareTypeName(const char* othername) {
        return 0 == strcmp(typeid(type).name(), othername);
    }

    template<class type>
    unsigned GetTypeSize() {
        return sizeof(type);
    }

    // We want void to be of indetermiante type for the queue
    template<>
    inline unsigned GetTypeSize<void>() { return 1; }

#define REGISTER_TYPE_NAME(type) \
    template<> inline const char* TypeName<type>() { return #type; } \
    template<> inline bool CompareTypeName<type>(const char* othername) { \
        return 0 == strcmp(#type, othername); \
    }

    REGISTER_TYPE_NAME(void);
    REGISTER_TYPE_NAME(char);
    REGISTER_TYPE_NAME(int8_t);
    REGISTER_TYPE_NAME(uint8_t);
    REGISTER_TYPE_NAME(int16_t);
    REGISTER_TYPE_NAME(uint16_t);
    REGISTER_TYPE_NAME(int32_t);
    REGISTER_TYPE_NAME(uint32_t);
    REGISTER_TYPE_NAME(int64_t);
    REGISTER_TYPE_NAME(uint64_t);
    REGISTER_TYPE_NAME(float);
    REGISTER_TYPE_NAME(double);
    REGISTER_TYPE_NAME(long double);

#undef REGISTER_TYPE_NAME

}

#endif

