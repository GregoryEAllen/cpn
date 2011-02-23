#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include <vector>
#include <stdint.h>

using namespace CPN;
using std::vector;
using std::string;

class Cons : public NodeBase {
public:
    Cons(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr) {}
private:
    void Process();
};

void Cons::Process() {
    int num_outputs = GetParam<int>("num outputs", 1);
    uint64_t current = GetParam<uint64_t>("initial");
    typedef vector< OQueue<uint64_t> > Outport_t;
    Outport_t outports;
    for (int i = 0; i < num_outputs; ++i) {
        std::ostringstream oss;
        oss << "out" << i;
        outports.push_back(GetOQueue(oss.str()));
    }
    IQueue<uint64_t> inport = GetIQueue("in");
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
