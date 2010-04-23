
#include "ForkNode.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include <complex>
#include <algorithm>

using CPN::shared_ptr;
using CPN::NodeBase;
using CPN::Kernel;
using CPN::NodeAttr;
using std::vector;
using CPN::QueueReaderAdapter;
using CPN::QueueWriterAdapter;
using std::for_each;
using std::mem_fun_ref;

CPN_DECLARE_NODE_FACTORY(ForkNodeType, ForkNode);

ForkNode::ForkNode(Kernel &ker, const NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    inport = param["inport"].AsString();
    for (Variant::ListIterator i = param["outports"].ListBegin();
            i != param["outports"].ListEnd(); ++i) {
        outports.push_back(i->AsString());
    }
    if (param["size"].IsNumber()) {
        size = param["size"].AsUnsigned();
    } else {
        size = 0;
    }
    if (param["overlap"].IsNumber()) {
        overlap = param["overlap"].AsUnsigned();
    } else {
        overlap = 0;
    }
}

void ForkNode::Process() {
    QueueReaderAdapter<void> in = GetReader(inport);
    vector<QueueWriterAdapter<void> > out(outports.size());
    vector<QueueWriterAdapter<void> >::iterator current = out.begin();
    for (vector<std::string>::iterator itr = outports.begin(); itr != outports.end(); ++itr, ++current)
        (*current) = GetWriter(*itr);

    const vector<QueueWriterAdapter<void> >::iterator end = out.end();
    bool loop = true;
    while (loop) {
        current = out.begin();
        while (current != end) {
            const void *inbuff = in.GetDequeuePtr(size);
            const unsigned chanstride = in.ChannelStride();
            const unsigned numchannels = in.NumChannels();
            if (!inbuff) {
                loop = false;
                break;
            }
            current->Enqueue(inbuff, size, numchannels, chanstride);
            in.Dequeue(size - overlap);
            ++current;
        }
    }
    in.Release();
    for_each(out.begin(), out.end(), mem_fun_ref(&QueueWriterAdapter<void>::Release));
}

