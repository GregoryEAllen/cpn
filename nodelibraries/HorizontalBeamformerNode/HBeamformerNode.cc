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
 * The parameters that this node takes are as follows:
 * file: a filename to load the beamformer from
 * estimate: Whether to pass estimate in default false.
 * half: a number 0, 1, 2 which tells which half of the
 * beamforming to do, 0 to do both halfs, 1 to do the first,
 * 2 to do the second. Default 0
 *
 * The input port is called "in" and the output port is called "out"
 */
#include "NodeBase.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "IQueue.h"
#include "OQueue.h"
#include "HBeamformer.h"
#include <complex>
#include <algorithm>
#include <functional>

using std::complex;

class HBeamformerNode : public CPN::NodeBase {
public:
    HBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr) {}
    ~HBeamformerNode() {}
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(HBeamformerNode, HBeamformerNode);

void HBeamformerNode::Process() {
    std::auto_ptr<HBeamformer> hbeam = HBLoadFromFile(GetParam("file"), GetParam<bool>("estimate", false));
    int half = GetParam<int>("half", 0);
    CPN::IQueue<complex<float> > in = GetIQueue("in");
    CPN::OQueue<complex<float> > out = GetOQueue("out");
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


