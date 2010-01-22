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
#include <stdio.h>

RemoteDatabase::~RemoteDatabase() {
}

void RemoteDatabase::SendMessage(const Variant &msg) {
    if (!Closed()) {
        std::string message = msg.AsJSON();
        //printf("<<< %s\n", message.c_str());
        Write(message.c_str(), message.size() + 1);
    }
}

void *RemoteDatabase::EntryPoint() {
    try {
        Readable(true);
        while (Good()) {
            Poll(-1);
        }
    } catch (const ErrnoException &e) {
        printf("RemoteDatabase: Uncaught errno exception (%d): %s\n", e.Error(), e.what());
        Terminate();
    }
    return 0;
}

void RemoteDatabase::OnRead() {
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
        for (unsigned i = 0; i < numread; ++i) {
            if (buf[i] == 0) {
                //buffer.push_back(buf[i]);
                //printf(">>> %s\n", &buffer[0]);
                Variant msg = Variant::FromJSON(buffer);
                DispatchMessage(msg);
                buffer.clear();
            } else {
                buffer.push_back(buf[i]);
            }
        }
    }
}

void RemoteDatabase::OnWrite() {
}

void RemoteDatabase::OnError() {
    Terminate();
    Close();
}

void RemoteDatabase::OnHup() {
    Terminate();
    Close();
}

void RemoteDatabase::OnInval() {
    Terminate();
}

