
#include "FanVBeamformerNode.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "FanVBeamformer.h"
#include "LoadFromFile.h"
#include <complex>
#include <algorithm>
#include <functional>

using std::complex;

CPN_DECLARE_NODE_FACTORY(FanVBeamformerNode, FanVBeamformerNode);

FanVBeamformerNode::FanVBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
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
    if (param["file"].IsString()) {
        std::auto_ptr<FanVBeamformer> vbf = FanVBLoadFromFile(param["file"].AsString());
        vbeam = vbf.release();
    } else {
        unsigned numFans = param["numFans"].AsUnsigned();
        unsigned numStaveTypes = param["numStaveTypes"].AsUnsigned();
        unsigned numElemsPerStave = param["numElemsPerStave"].AsUnsigned();
        unsigned filterLen = param["filterLen"].AsUnsigned();
        float *filter = (float*)attr.GetArg().GetBuffer();
        complex<float> *bbcor = (complex<float>*)(filter + filterLen * numStaveTypes * numElemsPerStave * numFans);
        vbeam = new FanVBeamformer(numFans, numStaveTypes, numElemsPerStave, filterLen,
                    filter, bbcor);
    }
    if (param["algorithm"].IsNumber()) {
        vbeam->SetAlgorithm(param["algorithm"].AsNumber<FanVBeamformer::Algorithm_t>());
    }

}

FanVBeamformerNode::~FanVBeamformerNode() {
    delete vbeam;
}


void FanVBeamformerNode::Process() {
    using std::vector;
    using std::string;
    using CPN::QueueWriterAdapter;
    using CPN::QueueReaderAdapter;
    QueueReaderAdapter<complex<short> > in = GetReader(inport);
    vector< QueueWriterAdapter< complex<float> > >::iterator out_itr;
    vector< QueueWriterAdapter< complex<float> > >::iterator out_end;
    vector< QueueWriterAdapter< complex<float> > > out(outports.size());
    vector< FanVBeamformer::ResVec > rv(outports.size());
    vector< FanVBeamformer::ResVec >::iterator rv_itr;
    out_itr = out.begin();
    for (vector<string>::iterator itr = outports.begin(); itr != outports.end(); ++itr)
        (*out_itr++) = GetWriter(*itr);
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


