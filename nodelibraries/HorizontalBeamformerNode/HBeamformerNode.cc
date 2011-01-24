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
#include "HBeamformerNode.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "IQueue.h"
#include "OQueue.h"
#include "HBeamformer.h"
#include <complex>
#include <algorithm>
#include <functional>

using std::complex;

CPN_DECLARE_NODE_FACTORY(HBeamformerNode, HBeamformerNode);

HBeamformerNode::HBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr), half(0)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    inport = param["inport"].AsString();
    outport = param["outport"].AsString();
    if (!param["half"].IsNull()) {
        half = param["half"].AsInt();
    }
    bool estimate = param["estimate"].AsBool();
    std::auto_ptr<HBeamformer> hbf = HBLoadFromFile(param["file"].AsString(), estimate);
    hbeam = hbf.release();
}

HBeamformerNode::~HBeamformerNode() {
    delete hbeam;
}

void HBeamformerNode::Process() {
    CPN::IQueue<complex<float> > in = GetReader(inport);
    CPN::OQueue<complex<float> > out = GetWriter(outport);
    if (half == 0) {
        ASSERT(out.NumChannels() == hbeam->NumBeams(),
                "%u != %u", out.NumChannels(), hbeam->NumBeams());
        while (true) {
            const complex<float> *inbuff = in.GetDequeuePtr(hbeam->Length());
            if (!inbuff) { break; }
            complex<float> *outbuff = out.GetEnqueuePtr(hbeam->Length());
            hbeam->Run(inbuff, in.ChannelStride(), outbuff, out.ChannelStride());
            in.Dequeue(hbeam->Length());
            out.Enqueue(hbeam->Length()); // change
        }
    } else if (half == 1) {
        const unsigned outlen = hbeam->NumVStaves() * hbeam->Length();
        while (true) {
            const complex<float> *inbuff = in.GetDequeuePtr(hbeam->Length());
            if (!inbuff) { break; }
            complex<float> *outbuff = out.GetEnqueuePtr(outlen);
            hbeam->RunFirstHalf(inbuff, in.ChannelStride(), outbuff);
            in.Dequeue(hbeam->Length());
            out.Enqueue(outlen);
        }
    } else if (half == 2) {
        ASSERT(out.NumChannels() == hbeam->NumBeams(),
                "%u != %u", out.NumChannels(), hbeam->NumBeams());
        unsigned inlen = hbeam->NumVStaves() * hbeam->Length();
        while (true) {
            const complex<float> *inbuff = in.GetDequeuePtr(inlen);
            if (!inbuff) { break; }
            complex<float> *outbuff = out.GetEnqueuePtr(hbeam->Length());
            hbeam->RunSecondHalf(inbuff, outbuff, out.ChannelStride());
            in.Dequeue(inlen);
            out.Enqueue(hbeam->Length()); // change
        }
    }
}


