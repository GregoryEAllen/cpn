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

#include "WakeupHandler.h"
#include "ErrnoException.h"
#include "Assert.h"
#include <unistd.h>

WakeupHandler::WakeupHandler()
    : wfd(-1)
{
    int filedes[2];
    if (pipe(filedes) != 0) {
        throw ErrnoException();
    }
    FD(filedes[0]);
    wfd = filedes[1];
    Readable(true);
    Writeable(false);
}

WakeupHandler::~WakeupHandler() {
    close(wfd);
    wfd = -1;
}

void WakeupHandler::SendWakeup() {
    char c = 0;
    int ret = 0;
    do {
        ret = write(wfd, &c, sizeof(c));
        if (ret < 0) {
            throw ErrnoException();
        }
    } while (ret != sizeof(c));
}

void WakeupHandler::OnRead() {
    char c[256];
    while (Read(c, sizeof(c)) != 0);
}

void WakeupHandler::OnWrite() {
    ASSERT(false, "Should be impossible read end of pipes");
}

void WakeupHandler::OnError() {
    Read(0,0);
}

void WakeupHandler::OnHup() {
    ASSERT(false, "Corrupted state");
}

void WakeupHandler::OnInval() {
    ASSERT(false, "Corrupted state");
}

