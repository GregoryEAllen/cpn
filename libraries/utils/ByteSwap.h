/** \file
 * \author John Bridgman
 * A nice little byteswapping template which uses inline asm
 * from libraries when it can.
 */
#pragma once

#ifdef OS_LINUX
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

