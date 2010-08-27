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
 * A nice little byteswapping template which uses inline asm
 * from libraries when it can.
 */
#ifndef BYTESWAP_H
#define BYTESWAP_H
#pragma once

#ifdef __linux__
#include <byteswap.h>
#endif

template <typename T>
inline T ByteSwap(T t) {
    T ret;
    switch (sizeof(T)) {
    case 1:
        ret = t;
        break;
#ifdef bswap_16
    case 2:
        ret = bswap_16(t);
        break;
#endif
#ifdef bswap_32
    case 4:
        ret = bswap_32(t);
        break;
#endif
#ifdef bswap_64
    case 8:
        ret = bswap_64(t);
        break;
#endif
    default:
        {
            char *in = (char*)&t, *out = (char*)&ret;
            for (int i = 0; i < sizeof(T); ++i) {
                out[i] = in[sizeof(T) - i - 1];
            }
        }
        break;
    }
    return ret;
}
#endif
