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

class ForkNode : public CPN::NodeBase {
public:
    ForkNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr) {}
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(ForkNode, ForkNode);


void ForkNode::Process() {
    unsigned num_outports = GetParam<unsigned>("num_outports", 1);
    unsigned size = GetParam<unsigned>("size", 0);
    unsigned overlap = GetParam<unsigned>("overlap", 0);

    IQueue<void> in = GetIQueue("in");
    vector<OQueue<void> > out;
    for (unsigned port = 0; port < num_outports; ++port) {
        std::ostringstream oss;
        oss << "out" << port;
        out.push_back(GetOQueue(oss.str()));
    }

    vector<OQueue<void> >::iterator current = out.begin();
    const vector<OQueue<void> >::iterator end = out.end();
    bool loop = true;
    while (loop) {
        current = out.begin();
        while (current != end) {
            const void *inbuff = in.GetDequeuePtr(size);
            const unsigned chanstride = in.ChannelStride();
            const unsigned numchannels = in.NumChannels();
            if (!inbuff) {
                loop = false;
                break;
            }
            current->Enqueue(inbuff, size, numchannels, chanstride);
            in.Dequeue(size - overlap);
            ++current;
        }
    }
    in.Release();
    for_each(out.begin(), out.end(), mem_fun_ref(&OQueue<void>::Release));
}

