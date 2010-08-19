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
#include "RemoteDatabase.h"
#include "ErrnoException.h"
#include "VariantToJSON.h"
#include "PthreadFunctional.h"
#include <stdio.h>

using std::auto_ptr;

RemoteDatabase::RemoteDatabase(const SocketAddress &addr)
{ 
    thread.reset(CreatePthreadFunctional(this, &RemoteDatabase::EntryPoint));
    Connect(addr);
    thread->Start();
}

RemoteDatabase::RemoteDatabase(const SockAddrList &addrs)
{ 
    thread.reset(CreatePthreadFunctional(this, &RemoteDatabase::EntryPoint));
    Connect(addrs);
    thread->Start();
}

RemoteDatabase::~RemoteDatabase() {
    thread->Join();
}

void RemoteDatabase::SendMessage(const Variant &msg) {
    if (!Closed()) {
        std::string message = VariantToJSON(msg);
        //printf("<<< %s\n", message.c_str());
        Write(message.data(), message.size());
    }
}

void *RemoteDatabase::EntryPoint() {
    try {
        Readable(false);
        while (Good() && !IsTerminated()) {
            Poll(-1);
            Read();
        }
    } catch (const ErrnoException &e) {
        printf("RemoteDatabase: Uncaught errno exception (%d): %s\n", e.Error(), e.what());
        Terminate();
    }
    Close();
    return 0;
}

void RemoteDatabase::Read() {
    const unsigned BUF_SIZE = 4*1024;
    char buf[BUF_SIZE];
    while (Good()) {
        unsigned numread = Recv(buf, BUF_SIZE, false);
        if (numread == 0) {
            if (Eof()) {
                Terminate();
                Close();
            }
            break;
        }
        unsigned numparsed = 0;
        while (numparsed < numread) {
            numparsed += parse.Parse(buf + numparsed, numread - numparsed);
            if (parse.Done()) {
                DispatchMessage(parse.Get());
                parse.Reset();
            } else if (parse.Error()) {
                printf("Error parsing input!\n");
                parse.Reset();
            }
        }
    }
}

