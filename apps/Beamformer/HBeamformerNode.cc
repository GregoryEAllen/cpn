
#include "HBeamformerNode.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "HBeamformer.h"
#include "LoadFromFile.h"
#include <complex>
#include <algorithm>
#include <functional>

using std::complex;

CPN_DECLARE_NODE_FACTORY(HBeamformerNodeTypeName, HBeamformerNode);

HBeamformerNode::HBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    inport = param["inport"].AsString();
    outport = param["outport"].AsString();
    bool estimate = param["estimate"].AsBool();
    if (param["file"].IsString()) {
        std::auto_ptr<HBeamformer> hbf = HBLoadFromFile(param["file"].AsString(), estimate);
        hbeam = hbf.release();
    } else {
        unsigned numStaves = param["numStaves"].AsUnsigned();
        unsigned numBeams = param["numBeams"].AsUnsigned();
        unsigned length = param["length"].AsUnsigned();
        ASSERT(attr.GetArg().GetSize() == (sizeof(complex<float>) * 2 * length * numBeams),
               "Wrong sized coefficients."); 
        complex<float> *coeffs = (complex<float>*)attr.GetArg().GetBuffer();
        complex<float> *replicas = coeffs + length * numBeams;
        std::vector<unsigned> staveIndexes(param["staveIndexes"].Size());
        std::transform(param["staveIndexes"].ListBegin(), param["staveIndexes"].ListEnd(),
                staveIndexes.begin(), std::mem_fun_ref(&Variant::AsUnsigned));
        hbeam = new HBeamformer(length, numStaves, numBeams, coeffs, replicas, &staveIndexes[0], estimate);
    }
}

HBeamformerNode::~HBeamformerNode() {
    delete hbeam;
}

void HBeamformerNode::Process() {
    CPN::QueueReaderAdapter<complex<float> > in = GetReader(inport);
    CPN::QueueWriterAdapter<complex<float> > out = GetWriter(outport);
    ASSERT(out.NumChannels() == hbeam->NumBeams());
    while (true) {
        const complex<float> *inbuff = in.GetDequeuePtr(hbeam->Length());
        if (!inbuff) { break; }
        complex<float> *outbuff = out.GetEnqueuePtr(hbeam->Length());
        hbeam->Run(inbuff, in.ChannelStride(), outbuff, out.ChannelStride());
        in.Dequeue(hbeam->Length());
        out.Enqueue(hbeam->Length()); // change
    }
}


