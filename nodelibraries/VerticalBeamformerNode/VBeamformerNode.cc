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
 * A CPN node to encapsulate a VBeamformer.
 */
#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include "VBeamformer.h"
#include <complex>
#include <vector>
#include <algorithm>
#include <functional>

using std::complex;

class VBeamformerNode : public CPN::NodeBase {
public:
    VBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr) {}
    ~VBeamformerNode() {}
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(VBeamformerNode, VBeamformerNode);


void VBeamformerNode::Process() {
    unsigned blocksize = GetParam<unsigned>("blocksize");
    unsigned num_outports = GetParam<unsigned>("num_outports");

    std::auto_ptr<VBeamformer> vbeam = VBLoadFromFile(GetParam("file"));

    if (HasParam("algorithm")) {
        int algo = GetParam<int>("algorithm");
        vbeam->SetAlgorithm((VBeamformer::Algorithm_t)algo);
    }

    CPN::IQueue<complex<short> > in = GetIQueue("in");
    std::vector< CPN::OQueue< complex<float> > >::iterator out_itr;
    std::vector< CPN::OQueue< complex<float> > >::iterator out_end;
    std::vector< CPN::OQueue< complex<float> > > out;

    for (unsigned port = 0; port < num_outports; ++port) {
        std::ostringstream oss;
        oss << "out" << port;
        out.push_back(GetOQueue(oss.str()));
    }

    out_itr = out.begin();
    out_end = out.end();
    const unsigned numstaves = in.NumChannels()/vbeam->NumElemsPerStave();
    while (true) {
        out_itr = out.begin();
        unsigned fan = 0;
        unsigned numsamples = blocksize + vbeam->FilterLen();
        const complex<short> *inptr = in.GetDequeuePtr(numsamples);
        if (!inptr) {
            break;
        }
        unsigned instride = in.ChannelStride();
        while (out_itr != out_end) {
            complex<float> *outptr = out_itr->GetEnqueuePtr(numsamples);
            unsigned outstride = out_itr->ChannelStride();
            unsigned numout = vbeam->Run(inptr, instride, numstaves, numsamples, fan, outptr, outstride);
            out_itr->Enqueue(numout);
            ++fan;
            ++out_itr;
        }
        in.Dequeue(blocksize);
    }
}


