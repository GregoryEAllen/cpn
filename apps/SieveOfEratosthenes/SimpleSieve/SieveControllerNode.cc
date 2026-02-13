/**
 * \brief SieveControllerNode implementation.
 */

#include "SieveControllerNode.h"
#include "FilterNode.h"
#include "ProducerNode.h"
#include "Kernel.h"
#include "IQueue.h"
#include "OQueue.h"
#include "NodeFactory.h"
#include "ToString.h"
#include "ThrowingAssert.h"
#include <cmath>
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

using CPN::NodeAttr;
using CPN::QueueAttr;

CPN_DECLARE_NODE_FACTORY(SieveControllerNode, SieveControllerNode);

SieveControllerNode::SieveControllerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr), lastprime(0) {}


static const char* const PRODUCER_NAME = "ProducerName";
static const char* const FILTER_FORMAT = "Filter: %lu";
static const char* const QUEUE_FORMAT = "Queue: %lu";
static const char* const IN_PORT = "x";
static const char* const OUT_PORT = "y";
static const char* const RESULT_IN = "p %lu";
static const char* const RESULT_PORT = "prodport";

void SieveControllerNode::Process(void) {
    DEBUG("SieveControllerNode %s start\n", GetName().c_str());
    Initialize();
    bool finaltransition = true;
    unsigned long readVal = 0;
    const unsigned long stopVal = (unsigned long)std::ceil(std::sqrt((double) primeBound));
    CPN::IQueue<unsigned long> in = GetIQueue(ToString(RESULT_IN, lastprime));
    CPN::OQueue<unsigned long> out = GetOQueue("output");
    do {
        in.Dequeue(&readVal, 1);
        DBPRINT("Result got %lu\n", readVal);
        if (readVal != 0) {
            out.Enqueue(&readVal, 1);
            if (readVal != lastprime) {
                if (readVal < stopVal) {
                    DBPRINT("Created new filter %lu\n", readVal);
                    CreateFilter(readVal);
                    in = GetIQueue(ToString(RESULT_IN, lastprime));
                } else if (finaltransition) {
                    QueueAttr qattr = GetQueueAttr();
                    qattr.SetWriter(ToString(FILTER_FORMAT, lastprime), OUT_PORT);
                    qattr.SetReader(GetName(), IN_PORT);
                    kernel.CreateQueue(qattr);
                    in = GetIQueue(IN_PORT);
                    finaltransition = false;
                }
            }
        }
    } while (readVal != 0 && primeBound > readVal);
    DEBUG("SieveControllerNode %s end\n", GetName().c_str());
}


void SieveControllerNode::Initialize(void) {

    primeBound = GetParam<unsigned long>("primeBound");
    numberBound = GetParam<unsigned long>("numberBound");
    queueSize = GetParam<unsigned long>("queueSize");

    NodeAttr producerattr(PRODUCER_NAME, SIEVE_PRODUCERNODE_TYPENAME);
    producerattr.SetParam("numberBound", numberBound);
    kernel.CreateNode(producerattr);

    lastprime = 2;
    std::string filtername = ToString(FILTER_FORMAT, lastprime);
    NodeAttr filterattr(filtername, SIEVE_FILTERNODE_TYPENAME);
    filterattr.SetParam("filterval", lastprime);
    kernel.CreateNode(filterattr);

    QueueAttr qattr = GetQueueAttr();
    qattr.SetWriter(PRODUCER_NAME, OUT_PORT);
    qattr.SetReader(filtername, IN_PORT);
    kernel.CreateQueue(qattr);

    qattr.SetWriter(filtername, RESULT_PORT);
    qattr.SetReader(GetName(), ToString(RESULT_IN, lastprime));
    kernel.CreateQueue(qattr);
}

void SieveControllerNode::CreateFilter(const unsigned long prime) {
    std::string nodename = ToString(FILTER_FORMAT, prime);
    NodeAttr attr(nodename, SIEVE_FILTERNODE_TYPENAME);
    attr.SetParam("filterval", prime);
    kernel.CreateNode(attr);

    QueueAttr qattr = GetQueueAttr();
    qattr.SetWriter(ToString(FILTER_FORMAT, lastprime), OUT_PORT);
    qattr.SetReader(nodename, IN_PORT);
    kernel.CreateQueue(qattr);

    qattr.SetWriter(nodename, RESULT_PORT);
    qattr.SetReader(GetName(), ToString(RESULT_IN, prime));
    kernel.CreateQueue(qattr);
    lastprime = prime;
}

QueueAttr SieveControllerNode::GetQueueAttr() {
    QueueAttr qattr(queueSize * sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    return qattr;
}


