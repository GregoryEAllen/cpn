
#include "VBeamformerNode.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "VBeamformer.h"
#include <complex>
#include <algorithm>
#include <functional>

using std::complex;

CPN_DECLARE_NODE_FACTORY(VBeamformerNodeTypeName, VBeamformerNode);

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
            outports.begin(), std::mem_fun_ref(&Variant::AsUnsigned));
    unsigned numFans = param["numFans"].AsUnsigned();
    unsigned numStaveTypes = param["numStaveTypes"].AsUnsigned();
    unsigned numElemsPerStave = param["numElemsPerStave"].AsUnsigned();
    unsigned filterLen = param["filterLen"].AsUnsigned();
    blocksize = param["blocksize"].AsUnsigned();
    short *filter = (short*)attr.GetArg().GetBuffer();
    complex<float> *bbcor = (complex<float>*)(filter + filterLen * numStaveTypes * numElemsPerStave * numFans);
    vbeam = new VBeamformer(numFans, numStaveTypes, numElemsPerStave, filterLen,
                filter, bbcor);
    if (param["algorithm"].IsNumber()) {
        vbeam->SetAlgorithm(param["algorithm"].AsNumber<VBeamformer::Algorithm_t>());
    }

}

VBeamformerNode::~VBeamformerNode() {
    delete vbeam;
}

void VBeamformerNode::Process() {
    CPN::QueueReaderAdapter<complex<short> > in = GetReader(inport);
    std::vector< CPN::QueueWriterAdapter< complex<float> > >::iterator out_itr;
    std::vector< CPN::QueueWriterAdapter< complex<float> > >::iterator out_end;
    std::vector< CPN::QueueWriterAdapter< complex<float> > > out(outports.size());
    out_itr = out.begin();
    for (std::vector<std::string>::iterator itr = outports.begin(); itr != outports.end(); ++itr)
        (*out_itr++) = GetWriter(*itr);
    out_end = out.end();
    const unsigned numinchannels = in.NumChannels();
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
            unsigned numout = vbeam->Run(inptr, instride, numinchannels, numsamples, fan, outptr, outstride);
            out_itr->Enqueue(numout);
            ++fan;
            ++out_itr;
        }
        in.Dequeue(blocksize);
    }
}


