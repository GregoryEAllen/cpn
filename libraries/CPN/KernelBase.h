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

#ifndef CPN_KERNELBASE_H
#define CPN_KERNELBASE_H
#pragma once
#include "CPNCommon.h"
#include "QueueAttr.h"
#include "NodeAttr.h"

namespace CPN {

    /**
     * Base class for the kernel.
     * This class contains all the methods that the database needs to call.
     * This class is primiarly used so that
     * the unit tests can provide a subclass to test the database functionality.
     */
    class CPN_API KernelBase {
    public:
        virtual ~KernelBase();
        virtual void RemoteCreateWriter(SimpleQueueAttr attr);
        virtual void RemoteCreateReader(SimpleQueueAttr attr);
        virtual void RemoteCreateQueue(SimpleQueueAttr attr);
        virtual void RemoteCreateNode(NodeAttr attr);
        virtual void NotifyTerminate();
    };
}

#endif
