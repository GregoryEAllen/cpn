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
 * A node which reads from one or more queues and then duplicates the input
 * to one or more outputs.
 */
#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

using CPN::shared_ptr;
using CPN::NodeBase;
using CPN::Kernel;
using CPN::NodeAttr;
using std::vector;
using CPN::IQueue;
using CPN::OQueue;
using std::for_each;
using std::mem_fun_ref;

class ForkJoinNode : public CPN::NodeBase {
public:
    ForkJoinNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr) {}
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(ForkJoinNode, ForkJoinNode);


void ForkJoinNode::Process() {
    unsigned num_inports = GetParam<unsigned>("num_inports", 1);
    unsigned num_outports = GetParam<unsigned>("num_outports", 1);
    unsigned size = GetParam<unsigned>("size", 0);
    unsigned overlap = GetParam<unsigned>("overlap", 0);

    vector<OQueue<void> > out;
    for (unsigned port = 0; port < num_outports; ++port) {
        std::ostringstream oss;
        oss << "out" << port;
        out.push_back(GetOQueue(oss.str()));
    }

    vector<IQueue<void> > in;
    for (unsigned port = 0; port < num_inports; ++port) {
        std::ostringstream oss;
        oss << "in" << port;
        in.push_back(GetIQueue(oss.str()));
    }
    vector<IQueue<void> >::iterator current_in = in.begin(), end_in = in.end();
    vector<OQueue<void> >::iterator current_out = out.begin(), end_out = out.end();
    while (true) {
        if (current_in == end_in) {
            current_in = in.begin();
        }
        if (current_out == end_out) {
            current_out = out.begin();
        }
        const void *inbuff = current_in->GetDequeuePtr(size);
        const unsigned chanstride = current_in->ChannelStride();
        const unsigned numchannels = current_in->NumChannels();
        if (!inbuff) {
            break;
        }
        current_out->Enqueue(inbuff, size, numchannels, chanstride);
        current_in->Dequeue(size - overlap);
        ++current_in;
        ++current_out;
    }
    for_each(out.begin(), out.end(), mem_fun_ref(&OQueue<void>::Release));
    for_each(in.begin(), in.end(), mem_fun_ref(&IQueue<void>::Release));
}
 
