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

#include "KernelConnectionHandler.h"

namespace CPN {
    KernelConnectionHandler::KernelConnectionHandler(KernelMessageHandler *kmh_)
        : kmh(kmh_)
    {
        logger.Output(kmh->GetLogger());
        logger.Name("ConnHan");
    }

    void KernelConnectionHandler::OnRead() {
        Sync::AutoReentrantLock arlock(lock);
    }

    void KernelConnectionHandler::OnError() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Error("An error occured on the kernel listen socket.");
    }

    void KernelConnectionHandler::OnInval() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Error("The kernel listen socke is invalid!?!?");
    }

    void KernelConnectionHandler::Register(std::vector<FileHandler*> &filehandlers) {
        Sync::AutoReentrantLock arlock(lock);
        if (!Closed()) {
            filehandlers.push_back(this);
        }
    }

    void KernelConnectionHandler::Shutdown() {
        Sync::AutoReentrantLock arlock(lock);
    }

    shared_ptr<Future<int> > KernelConnectionHandler::GetReaderDescriptor(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arlock(lock);
        ASSERT(false);
    }

    shared_ptr<Future<int> > KernelConnectionHandler::GetWriterDescriptor(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arlock(lock);
        ASSERT(false);
    }
}
