/** \file
 * \brief ProducerNode implementation
 */

#include "ProducerNode.h"
#include "NodeFactory.h"
#include "OQueue.h"
#include "Kernel.h"
#include "Assert.h"
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

ProducerNode::ProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr) {}

CPN_DECLARE_NODE_FACTORY(SieveProducerNode, ProducerNode);

void ProducerNode::Process(void) {
    DEBUG("ProducerNode %s start\n", GetName().c_str());
    const unsigned long numberBound = GetParam<unsigned long>("numberBound");
    CPN::OQueue<unsigned long> out = GetOQueue("y");
    const unsigned long cutoff = numberBound;
    unsigned long counter = 2;
    while (cutoff == 0 || counter < cutoff) {
        DBPRINT("ProducerNode: enqueue(%lu)\n", counter);
        out.Enqueue(&counter, 1);
        ++counter;
    }
    // Ending marker
    counter = 0;
    out.Enqueue(&counter, 1);
    DEBUG("ProducerNode %s end\n", GetName().c_str());
}

