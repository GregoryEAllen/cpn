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

#include "CPNCommon.h"
#include <stdint.h>
#include <typeinfo>
#include <complex>
#include <string>

namespace CPN {

    bool TypeCompatable(const std::string &type1, const std::string &type2);

    template<class type>
    std::string TypeName() {
        return typeid(type).name();
    }

    // this function is so that void has a size of 1
    template<class type>
    unsigned GetTypeSize() {
        return sizeof(type);
    }

    // We want void to be of indetermiante type for the queue
    template<>
    inline unsigned GetTypeSize<void>() { return 1; }


#define REGISTER_TYPE(type, name) \
    template<> inline std::string TypeName<type>() { return std::string(name); }
#define REGISTER_TYPE_NAME(type) REGISTER_TYPE(type, #type)

    REGISTER_TYPE_NAME(void);
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

    REGISTER_TYPE(std::complex<int8_t>, "complex<int8_t>");
    REGISTER_TYPE(std::complex<uint8_t>, "complex<uint8_t>");
    REGISTER_TYPE(std::complex<int16_t>, "complex<int16_t>");
    REGISTER_TYPE(std::complex<uint16_t>, "complex<uint16_t>");
    REGISTER_TYPE(std::complex<int32_t>, "complex<int32_t>");
    REGISTER_TYPE(std::complex<uint32_t>, "complex<uint32_t>");
    REGISTER_TYPE(std::complex<int64_t>, "complex<int64_t>");
    REGISTER_TYPE(std::complex<uint64_t>, "complex<uint64_t>");
    REGISTER_TYPE(std::complex<float>, "complex<float>");
    REGISTER_TYPE(std::complex<double>, "complex<double>");
    REGISTER_TYPE(std::complex<long double>, "complex<long double>");

#undef REGISTER_TYPE_NAME
#undef REGISTER_TYPE

}

#endif

