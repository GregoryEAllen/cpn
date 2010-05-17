
#include "HBeamformerNode.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "HBeamformer.h"

using std::complex;

HBeamformerNode::HBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    inport = param["inport"].AsString();
    outport = param["outport"].AsString();
}

HBeamformerNode::~HBeamformerNode() {
    delete hbeam;
}

void HBeamformerNode::Process() {
    CPN::QueueReaderAdapter<complex<float> > in = GetReader(inport);
    CPN::QueueWriterAdapter<complex<float> > out = GetWriter(outport);
    while (true) {
        const complex<float> *inbuff = in.GetDequeuePtr(hbeam->Length());
        if (!inbuff) { break; }
        complex<float> *outbuff = out.GetEnqueuePtr(hbeam->Length());
        hbeam->Run(inbuff, in.ChannelStride(), outbuff, out.ChannelStride());
        in.Dequeue(hbeam->Length());
        out.Enqueue(hbeam->Length()); // change
    }
}


