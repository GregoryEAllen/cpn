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
#include "RemoteContext.h"
#include "ErrnoException.h"
#include "VariantToJSON.h"
#include "PthreadFunctional.h"
#include <stdio.h>

using std::auto_ptr;

RemoteContext::RemoteContext(const SocketAddress &addr)
    : endwrite(false)
{ 
    thread.reset(CreatePthreadFunctional(this, &RemoteContext::EntryPoint));
    sock.Connect(addr);
    sock.SetNoDelay(true);
    thread->Start();
}

RemoteContext::RemoteContext(const SockAddrList &addrs)
    : endwrite(false)
{ 
    thread.reset(CreatePthreadFunctional(this, &RemoteContext::EntryPoint));
    sock.Connect(addrs);
    sock.SetNoDelay(true);
    thread->Start();
}

RemoteContext::~RemoteContext() {
    EndWrite();
    thread->Join();
}

void RemoteContext::SendMessage(const Variant &msg) {
    // We have the lock
    if (!sock.Closed() && !endwrite) {
        std::string message = VariantToJSON(msg);
        //printf("<<< %s\n", message.c_str());
        sock.Write(message.data(), message.size());
    }
}

void *RemoteContext::EntryPoint() {
    const unsigned BUF_SIZE = 4*1024;
    char buf[BUF_SIZE];
    try {
        sock.Readable(false);
        sock.Poll(-1);
        while (sock.Good()) {
            unsigned numread = sock.Recv(buf, BUF_SIZE, false);
            if (numread == 0) {
                if (sock.Eof()) {
                    Terminate();
                    return 0;
                }
                sock.Poll(-1);
            } else {
                unsigned numparsed = 0;
                while (numparsed < numread) {
                    numparsed += parse.Parse(buf + numparsed, numread - numparsed);
                    if (parse.Done()) {
                        DispatchMessage(parse.Get());
                        parse.Reset();
                    } else if (parse.Error()) {
                        fprintf(stderr, "RemoteContext: Error parsing input!\n");
                        parse.Reset();
                    }
                }
            }
        }
    } catch (const ErrnoException &e) {
        fprintf(stderr, "RemoteContext: Uncaught errno exception (%d): %s\n", e.Error(), e.what());
        Terminate();
    }
    sock.Close();
    return 0;
}

void RemoteContext::EndWrite() {
    PthreadMutexProtected alock(lock);
    endwrite = true;
    sock.ShutdownWrite();
}

bool RemoteContext::IsEndWrite() {
    PthreadMutexProtected alock(lock);
    return endwrite;
}

