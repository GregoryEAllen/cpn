#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include "JSONToVariant.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <functional>
#include <stdint.h>

using namespace CPN;
using std::vector;
using std::string;

class Summer : public NodeBase {
public:
    Summer(Kernel &ker, const NodeAttr &attr);
private:
    void Process();
    vector<string> inputs;
    string output;
};

Summer::Summer(Kernel &ker, const NodeAttr &attr)
    : NodeBase(ker, attr)
{
    JSONToVariant p;
    p.Parse(attr.GetParam());
    if (!p.Done()) {
        throw std::runtime_error("Could not parse parameters");
    }
    Variant param = p.Get();
    Variant inp = param["inputs"];
    std::transform(inp.ListBegin(), inp.ListEnd(),
            std::back_inserter(inputs),
            std::mem_fun_ref(&Variant::AsString));
    output = param["output"].AsString();
}

void Summer::Process() {
    typedef vector< IQueue<uint64_t> > Inport_t;
    Inport_t inports;
    for (vector<string>::iterator i = inputs.begin(), e = inputs.end();
            i != e; ++i) {
        inports.push_back(GetReader(*i));
    }
    OQueue<uint64_t> outport = GetWriter(output);
    bool loop = true;
    while (true) {
        uint64_t sum = 0;
        for (Inport_t::iterator i = inports.begin(), e = inports.end();
                i != e; ++i) {
            uint64_t val = 0;
            if (i->Dequeue(&val, 1)) {
                sum += val;
            } else {
                loop = false;
                break;
            }
        }
        if (!loop) break;
        outport.Enqueue(&sum, 1);
    }
}

CPN_DECLARE_NODE_FACTORY(Summer, Summer);
