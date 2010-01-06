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

#ifndef CPN_MESSAGE_H
#define CPN_MESSAGE_H
#pragma once
#include "CPNCommon.h"
#include "Assert.h"
#include "Future.h"
#include "Logger.h"

namespace CPN {

    class KernelMessageHandler {
    public:
        virtual ~KernelMessageHandler();
        virtual void CreateWriter(Key_t dst, const SimpleQueueAttr &attr);
        virtual void CreateReader(Key_t dst, const SimpleQueueAttr &attr);
        virtual void CreateQueue(Key_t dst, const SimpleQueueAttr &attr);
        virtual void CreateNode(Key_t dst, const NodeAttr &attr);

        virtual const LoggerOutput *GetLogger() const = 0;
        virtual shared_ptr<Database> GetDatabase() const = 0;

        // Functions the streams need of the kernel
        virtual shared_ptr<Future<int> > GetReaderDescriptor(Key_t readerkey, Key_t writerkey);
        virtual shared_ptr<Future<int> > GetWriterDescriptor(Key_t readerkey, Key_t writerkey);
        virtual void SendWakeup();
    };
}

#endif
