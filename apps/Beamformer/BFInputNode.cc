
#include "BFInputNode.h"
#include "OQueue.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include <fstream>

CPN_DECLARE_NODE_FACTORY(BFInputNodeTypeName, BFInputNode);

BFInputNode::BFInputNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant parser;
    parser.Parse(attr.GetParam().data(), attr.GetParam().size());
    ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
    Variant param = parser.Get();
    outport = param["outport"].AsString();
    if (param["infile"].IsString()) {
        infiles.push_back(param["infile"].AsString());
    }
    if (param["infiles"].IsArray()) {
        for (Variant::ListIterator i = param["infiles"].ListBegin();
                i != param["infiles"].ListEnd(); ++i) {
            infiles.push_back(i->AsString());
        }
    }
    repetitions = 1;
    if (param["repetitions"].IsNumber()) {
        repetitions = param["repetitions"].AsUnsigned();
    }
}

void BFInputNode::Process() {
    CPN::OQueue<void> out = GetWriter(outport);
    for (unsigned rep = 0; rep < repetitions; ++rep) {
        for (std::vector<std::string>::iterator i = infiles.begin();
                i != infiles.end(); ++i)
        {
            std::ifstream in(i->c_str());
            while (true) {
                JSONToVariant parser;
                in >> parser;
                if (!in.good() || !parser.Done()) {
                    // either an error or we are at the end of file
                    // Just ignore errors for now...
                    break;
                }
                Variant header = parser.Get();
                while (in.get() != 0 && in.good());
                const unsigned length = header["length"].AsUnsigned();
                const unsigned numChans = header["numChans"].AsUnsigned();
                for (unsigned i = 0; i < numChans; ++i) {
                    void *ptr = out.GetEnqueuePtr(length, i);
                    unsigned numread = 0;
                    while (in.good() && numread < length) {
                        in.read((char*)ptr + numread, length - numread);
                        numread += in.gcount();
                    }
                }
                out.Enqueue(length);
            }
        }
    }
}
