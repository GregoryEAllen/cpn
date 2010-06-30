
#include "NullNode.h"
#include "QueueReaderAdapter.h"

CPN_DECLARE_NODE_FACTORY(NullNode, NullNode);

NullNode::NullNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr), inport(attr.GetParam())
{
}

void NullNode::Process() {
    CPN::QueueReaderAdapter<void> in = GetReader(inport);
    while (true) {
        const unsigned maxthresh = in.MaxThreshold();
        if (!in.GetDequeuePtr(maxthresh)) {
            break;
        }
        in.Dequeue(maxthresh);
    }
}
