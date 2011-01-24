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
#include "VBeamformerNode.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "IQueue.h"
#include "OQueue.h"
#include "VBeamformer.h"
#include <complex>
#include <algorithm>
#include <functional>

using std::complex;

CPN_DECLARE_NODE_FACTORY(VBeamformerNode, VBeamformerNode);

VBeamformerNode::VBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    inport = param["inport"].AsString();
    outports.resize(param["outports"].Size());
    std::transform(param["outports"].ListBegin(), param["outports"].ListEnd(),
            outports.begin(), std::mem_fun_ref(&Variant::AsString));
    blocksize = param["blocksize"].AsUnsigned();
    std::auto_ptr<VBeamformer> vbf = VBLoadFromFile(param["file"].AsString());
    vbeam = vbf.release();
    if (param["algorithm"].IsNumber()) {
        vbeam->SetAlgorithm(param["algorithm"].AsNumber<VBeamformer::Algorithm_t>());
    }

}

VBeamformerNode::~VBeamformerNode() {
    delete vbeam;
}

void VBeamformerNode::Process() {
    CPN::IQueue<complex<short> > in = GetReader(inport);
    std::vector< CPN::OQueue< complex<float> > >::iterator out_itr;
    std::vector< CPN::OQueue< complex<float> > >::iterator out_end;
    std::vector< CPN::OQueue< complex<float> > > out(outports.size());
    out_itr = out.begin();
    for (std::vector<std::string>::iterator itr = outports.begin(); itr != outports.end(); ++itr)
        (*out_itr++) = GetWriter(*itr);
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


