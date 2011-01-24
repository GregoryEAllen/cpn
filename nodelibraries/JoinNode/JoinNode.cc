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
#include "JoinNode.h"
#include "IQueue.h"
#include "OQueue.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include <complex>
#include <algorithm>

using CPN::shared_ptr;
using CPN::NodeBase;
using CPN::Kernel;
using CPN::NodeAttr;
using std::vector;
using CPN::IQueue;
using CPN::OQueue;
using std::for_each;
using std::mem_fun_ref;

CPN_DECLARE_NODE_FACTORY(JoinNode, JoinNode);

JoinNode::JoinNode(Kernel &ker, const NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    outport = param["outport"].AsString();
    for (Variant::ListIterator i = param["inports"].ListBegin();
            i != param["inports"].ListEnd(); ++i) {
        inports.push_back(i->AsString());
    }
    if (param["size"].IsNumber()) {
        size = param["size"].AsUnsigned();
    } else {
        size = 0;
    }
    if (param["overlap"].IsNumber()) {
        overlap = param["overlap"].AsUnsigned();
    } else {
        overlap = 0;
    }
}

void JoinNode::Process() {
    OQueue<void> out = GetWriter(outport);
    vector<IQueue<void> > in(inports.size());
    vector<IQueue<void> >::iterator current = in.begin();
    for (vector<std::string>::iterator itr = inports.begin(); itr != inports.end(); ++itr)
        *(current++) = GetReader(*itr);

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
