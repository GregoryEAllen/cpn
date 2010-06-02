
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

}

VBeamformerNode::~VBeamformerNode() {
    delete vbeam;
}

void VBeamformerNode::Process() {
    CPN::QueueReaderAdapter<complex<float> > in = GetReader(inport);
    CPN::QueueWriterAdapter<complex<float> > out;
    while (true) {
    }
}


