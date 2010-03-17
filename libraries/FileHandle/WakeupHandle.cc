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

#include "WakeupHandle.h"
#include "ErrnoException.h"
#include "Assert.h"
#include <unistd.h>
#include <errno.h>

WakeupHandle::WakeupHandle()
    : wfd(-1)
{
    int filedes[2];
    if (pipe(filedes) != 0) {
        throw ErrnoException();
    }
    FD(filedes[0]);
    wfd = filedes[1];
    Readable(false);
    Writeable(false);
    SetBlocking(false);
    SetBlocking(wfd, false);
}

WakeupHandle::~WakeupHandle() {
    close(wfd);
    wfd = -1;
}

void WakeupHandle::SendWakeup() {
    int wfiled;
    {
        FileHandle::AutoLock al(file_lock);
        wfiled = wfd;
    }
    char c = 0;
    int ret = 0;
    do {
        ret = write(wfiled, &c, sizeof(c));
        if (ret < 0) {
            if (errno == EAGAIN) {
                // The buffer is full.
                break;
            }
            throw ErrnoException();
        }
    } while (ret != sizeof(c));
}

void WakeupHandle::Read() {
    char c[256];
    unsigned ret = 0;
    // If we didn't read all the buffer there isn't more to read
    do {
        ret = FileHandle::Read(c, sizeof(c));
    } while (ret == sizeof(c));
}

