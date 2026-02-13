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
#include "FanVBeamformer.h"
#include <complex>
#include <algorithm>
#include <vector>
#include <functional>

using std::complex;
using std::vector;
using std::string;
using CPN::OQueue;
using CPN::IQueue;

class FanVBeamformerNode : public CPN::NodeBase {
public:
    FanVBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr) {}
    ~FanVBeamformerNode() {}
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(FanVBeamformerNode, FanVBeamformerNode);

void FanVBeamformerNode::Process() {
    unsigned num_outports = GetParam<unsigned>("num_outports");
    unsigned blocksize = GetParam<unsigned>("blocksize");

    std::auto_ptr<FanVBeamformer> vbeam = FanVBLoadFromFile(GetParam("file"));
    if (HasParam("algorithm")) {
        int algo = GetParam<int>("algorithm");
        vbeam->SetAlgorithm((FanVBeamformer::Algorithm_t)algo);
    }

    IQueue<complex<short> > in = GetIQueue("in");
    vector< OQueue< complex<float> > >::iterator out_itr;
    vector< OQueue< complex<float> > >::iterator out_end;
    vector< OQueue< complex<float> > > out;

    for (unsigned port = 0; port < num_outports; ++port) {
        std::ostringstream oss;
        oss << "out" << port;
        out.push_back(GetOQueue(oss.str()));
    }

    vector< FanVBeamformer::ResVec > rv(out.size());
    vector< FanVBeamformer::ResVec >::iterator rv_itr;

    out_itr = out.begin();
    out_end = out.end();
    const unsigned numstaves = in.NumChannels()/vbeam->NumElemsPerStave();
    while (true) {
        out_itr = out.begin();
        rv_itr = rv.begin();
        unsigned fan = 0;
        unsigned numsamples = blocksize + vbeam->FilterLen();
        const complex<short> *inptr = in.GetDequeuePtr(numsamples);
        if (!inptr) {
            numsamples = in.Count();
            if (numsamples <= vbeam->FilterLen()) {
                break;
            }
            inptr = in.GetDequeuePtr(numsamples);
        }
        unsigned instride = in.ChannelStride();
        while (out_itr != out_end) {
            rv_itr->outdata = out_itr->GetEnqueuePtr(numsamples);
            rv_itr->outstride = out_itr->ChannelStride();
            rv_itr->fan = fan;
            ++fan;
            ++out_itr;
            ++rv_itr;
        }
        unsigned numout = vbeam->Run(inptr, instride, numstaves, numsamples, &rv[0], rv.size());
        in.Dequeue(numout);
        out_itr = out.begin();
        while (out_itr != out_end) {
            out_itr->Enqueue(numout);
            ++out_itr;
        }
    }
}


