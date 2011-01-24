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

#include "ForkNode.h"
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

CPN_DECLARE_NODE_FACTORY(ForkNode, ForkNode);

ForkNode::ForkNode(Kernel &ker, const NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    inport = param["inport"].AsString();
    for (Variant::ListIterator i = param["outports"].ListBegin();
            i != param["outports"].ListEnd(); ++i) {
        outports.push_back(i->AsString());
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

void ForkNode::Process() {
    IQueue<void> in = GetReader(inport);
    vector<OQueue<void> > out(outports.size());
    vector<OQueue<void> >::iterator current = out.begin();
    for (vector<std::string>::iterator itr = outports.begin(); itr != outports.end(); ++itr, ++current)
        (*current) = GetWriter(*itr);

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

