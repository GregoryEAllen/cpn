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
 * The KernelStream encapsulates the stream connection
 * to other kernels. It takes care of translating KernelMessage
 * to the stream and back.
 * \author John Bridgman
 */

#ifndef CPN_KERNELSTREAM_H
#define CPN_KERNELSTREAM_H
#pragma once

#include "CPNCommon.h"
#include "KernelMessage.h"
#include "CPNStream.h"
#include "AsyncStream.h"

#include <sigc++/sigc++.h>
#include <vector>

namespace CPN {

    class KernelStream : public KMsgDispatchable {
    public:

        KernelStream(
                Async::DescriptorPtr desc,
                sigc::slot<void> wake,
                sigc::slot<void, KernelMessagePtr> msgq);

        void ProcessMessage(KMsgCreateWriter *msg);
        void ProcessMessage(KMsgCreateReader *msg);
        void ProcessMessage(KMsgCreateQueue *msg);
        void ProcessMessage(KMsgCreateNode *msg);

        bool Connected();
        void RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors);
        void RunOneIteration();

        Key_t GetKey() { return hostkey; }
    private:

        void InitDescriptor();

        bool Readable();
        bool Writeable();
        void ReadReady();
        void WriteReady();

        Key_t hostkey;
        bool connected;
        Async::DescriptorPtr descriptor;

        Kernel *kernel;

        sigc::signal<void> wakeup;
        sigc::signal<void, KernelMessagePtr> enqueuemsg;
    };

}
#endif

