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

class Cons : public NodeBase {
public:
    Cons(Kernel &ker, const NodeAttr &attr);
private:
    void Process();
    string input;
    vector<string> outputs;
    uint64_t current;
};

Cons::Cons(Kernel &ker, const NodeAttr &attr)
    : NodeBase(ker, attr)
{
    JSONToVariant p;
    p.Parse(attr.GetParam());
    if (!p.Done()) {
        throw std::runtime_error("Could not parse parameters");
    }
    Variant param = p.Get();
    Variant outp = param["outputs"];
    std::transform(outp.ListBegin(), outp.ListEnd(),
            std::back_inserter(outputs),
            std::mem_fun_ref(&Variant::AsString));
    input = param["input"].AsString();
    current = param["initial"].AsNumber<uint64_t>();
}

void Cons::Process() {
    typedef vector< OQueue<uint64_t> > Outport_t;
    Outport_t outports;
    for (vector<string>::iterator i = outputs.begin(), e = outputs.end();
            i != e; ++i) {
        outports.push_back(GetWriter(*i));
    }
    IQueue<uint64_t> inport = GetReader(input);
    while (true) {
        for (Outport_t::iterator i = outports.begin(), e = outports.end();
                i != e; ++i) {
            i->Enqueue(&current, 1);
        }
        if (!inport.Dequeue(&current, 1)) {
            break;
        }
    }
}

CPN_DECLARE_NODE_FACTORY(Cons, Cons);
