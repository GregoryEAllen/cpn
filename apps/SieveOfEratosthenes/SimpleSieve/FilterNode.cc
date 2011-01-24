/** \file
 * \brief FilterNode implementation.
 */

#include "FilterNode.h"
#include "Kernel.h"
#include "OQueue.h"
#include "IQueue.h"
#include "NodeFactory.h"
#include "JSONToVariant.h"
#include <stdexcept>

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#if 0
#define DBPRINT(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DBPRINT(frmt, ...)
#endif
#else
#define DEBUG(frmt, ...)
#define DBPRINT(frmt, ...)
#endif

CPN_DECLARE_NODE_FACTORY(SieveFilterNode, FilterNode);

FilterNode::FilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr)
{
    JSONToVariant p;
    p.Parse(attr.GetParam());
    ASSERT(p.Done());
    Variant param = p.Get();
    filterval = param["filterval"].AsNumber<unsigned long>();
    threshold = param["filterval"].AsNumber<unsigned long>();
}

void FilterNode::Process(void) {
    DEBUG("FilterNode %s start\n", GetName().c_str());
    CPN::OQueue<unsigned long> out = GetWriter("prodport");
    CPN::IQueue<unsigned long> in = GetReader("x");
    bool sendtoresult = true;
    unsigned long nextFilter = filterval;
    unsigned long readVal = 0;
    do {
        in.Dequeue(&readVal, 1);
        DBPRINT("Filter %s got %lu\n", GetName().c_str(), readVal);
        if (readVal == filterval) {
            DBPRINT("Filter %s passed on %lu\n", GetName().c_str(), readVal);
            out.Enqueue(&readVal, 1);
        } else if (readVal == nextFilter) {
            nextFilter += filterval;
        } else {
            if (readVal > nextFilter) {
                nextFilter += filterval;
            }
            DBPRINT("Filter %s put %lu\n", GetName().c_str(), readVal);
            out.Enqueue(&readVal, 1);
            if (sendtoresult) {
                out = GetWriter("y");
                sendtoresult = false;
            }
        }
    } while (readVal != 0);
    DEBUG("FilterNode %s end\n", GetName().c_str());
}


