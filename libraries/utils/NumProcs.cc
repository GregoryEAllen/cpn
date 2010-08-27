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
#include "NumProcs.h"
#include "ErrnoException.h"
#include <sched.h>

#ifdef __linux__

void SetNumProcs(int n) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < n; ++i) {
        CPU_SET(i, &mask);
    }
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        throw ErrnoException();
    }
}

int GetNumProcs() {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    if (sched_getaffinity(0, sizeof(mask), &mask) == -1) {
        throw ErrnoException();
    }
    int num = 0;
    for (int i = 0; i < CPU_SETSIZE; ++i) {
        if (CPU_ISSET(i, &mask)) {
            ++num;
        }
    }
    return num;
}

#else

#include "SysConf.h"
#include <errno.h>

void SetNumProcs(int) {
    throw ErrnoException(ENOSYS);
}

int GetNumProcs() {
    return NumProcessorsOnline();
}

#endif
