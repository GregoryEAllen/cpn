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
 * The JoinNode reads from the inputs in a round robbin fashion and
 * then writes to the output, optionally discarding some portion read.
 */
#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
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

class JoinNode : public CPN::NodeBase {
public:
    JoinNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr) {}
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(JoinNode, JoinNode);

void JoinNode::Process() {
    unsigned num_inputs = GetParam<unsigned>("num_inports", 1);
    unsigned size = GetParam<unsigned>("size", 0);
    unsigned overlap = GetParam<unsigned>("overlap", 0);

    OQueue<void> out = GetOQueue("out");
    vector<IQueue<void> > in;
    for (unsigned port = 0; port < num_inputs; ++port) {
        std::ostringstream oss;
        oss << "in" << port;
        in.push_back(GetIQueue(oss.str()));
    }

    vector<IQueue<void> >::iterator current = in.begin();
    const vector<IQueue<void> >::iterator end = in.end();
    bool loop = true;
    while (loop) {
        current = in.begin();
        while (current != end) {
            const void *inbuff = current->GetDequeuePtr(size);
            const unsigned chanstride = current->ChannelStride();
            const unsigned numchannels = current->NumChannels();
            if (!inbuff) {
                loop = false;
                break;
            }
            out.Enqueue(inbuff, size, numchannels, chanstride);
            current->Dequeue(size - overlap);
            ++current;
        }
    }
    out.Release();
    for_each(in.begin(), in.end(), mem_fun_ref(&IQueue<void>::Release));
}
