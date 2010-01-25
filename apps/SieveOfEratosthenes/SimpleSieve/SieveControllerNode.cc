/**
 * \brief SieveControllerNode implementation.
 */

#include "SieveControllerNode.h"
#include "FilterNode.h"
#include "ProducerNode.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "NodeFactory.h"
#include "ToString.h"
#include "Assert.h"
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

class SCNFactory : public CPN::NodeFactory {
public:
	SCNFactory() : CPN::NodeFactory(SIEVECONTROLLERNODE_TYPENAME) {}

    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel& ker, const CPN::NodeAttr &attr) {
        ASSERT(attr.GetArg().GetSize() == sizeof(SieveControllerNode::Param));
        return CPN::shared_ptr<CPN::NodeBase>(
                new SieveControllerNode(ker, attr,
                    *((SieveControllerNode::Param*)attr.GetArg().GetBuffer())));
	}

};

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
	const unsigned long stopVal = (unsigned long)std::ceil(std::sqrt((double) param.primeBound));
	CPN::QueueReaderAdapter<unsigned long> in = GetReader(ToString(RESULT_IN, lastprime));
	do {
		in.Dequeue(&readVal, 1);
        DBPRINT("Result got %lu\n", readVal);
		if (readVal != 0) {
			param.results->push_back(readVal);
            if (readVal != lastprime) {
                if (readVal < stopVal) {
                    DBPRINT("Created new filter %lu\n", readVal);
                    CreateFilter(readVal);
                    in = GetReader(ToString(RESULT_IN, lastprime));
                } else if (finaltransition) {
                    QueueAttr qattr = GetQueueAttr();
                    qattr.SetWriter(ToString(FILTER_FORMAT, lastprime), OUT_PORT);
                    qattr.SetReader(GetName(), IN_PORT);
                    kernel.CreateQueue(qattr);
                    in = GetReader(IN_PORT);
                    finaltransition = false;
                }
            }
		}
	} while (readVal != 0 && param.primeBound > readVal);
	kernel.Terminate();
	DEBUG("SieveControllerNode %s end\n", GetName().c_str());
}

extern "C" {
    CPN::shared_ptr<CPN::NodeFactory> cpninitSieveControllerNodeTypeName(void);
}

CPN::shared_ptr<CPN::NodeFactory> cpninitSieveControllerNodeTypeName(void) {
	return (CPN::shared_ptr<CPN::NodeFactory>(new SCNFactory));
}

void SieveControllerNode::Initialize(void) {

    NodeAttr producerattr(PRODUCER_NAME, SIEVE_PRODUCERNODE_TYPENAME);
    producerattr.SetParam(StaticConstBuffer(&param, sizeof(param)));
	kernel.CreateNode(producerattr);

    lastprime = 2;
    std::string filtername = ToString(FILTER_FORMAT, lastprime);
    NodeAttr filterattr(filtername, SIEVE_FILTERNODE_TYPENAME);
	FilterNode::Param p = { lastprime, param.threshold };
    filterattr.SetParam(StaticConstBuffer(&p, sizeof(p)));
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
	FilterNode::Param p = { prime, param.threshold };
    attr.SetParam(StaticConstBuffer(&p, sizeof(p)));
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
    QueueAttr qattr(param.queueSize * sizeof(unsigned long),
            param.threshold * sizeof(unsigned long));
    qattr.SetHint(param.queuehint).SetDatatype<unsigned long>();
    return qattr;
}


