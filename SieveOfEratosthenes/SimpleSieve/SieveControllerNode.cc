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
#include <cmath>

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif


class SCNFactory : public CPN::NodeFactory {
public:
	SCNFactory() : CPN::NodeFactory(SIEVECONTROLLERNODE_TYPENAME) {}

	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr &attr,
		       	const void* const arg, const CPN::ulong argsize) {
		return new SieveControllerNode(ker, attr,
				*((SieveControllerNode::Param*)arg));
	}

	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};

static SCNFactory scnFactoryInstance;
static const char* const PRODUCER_NAME = "ProducerName";
static const char* const FILTER_FORMAT = "Filter: %lu";
static const char* const QUEUE_FORMAT = "Queue: %lu";
static const char* const IN_PORT = "x";
static const char* const OUT_PORT = "y";

void SieveControllerNode::Process(void) {
	DEBUG("SieveControllerNode %s start\n", GetName().c_str());
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(GetName(), IN_PORT);
	Initialize();
	unsigned long readVal = 0;
	unsigned long stopVal = (unsigned long)ceil(sqrt((double) param.primeBound));
	do {
		in.Dequeue(&readVal, 1);
		if (readVal != 0) {
			param.results->push_back(readVal);
			if (readVal < stopVal) {
				CreateFilter(readVal);
			}
		}
	} while (readVal != 0 && param.primeBound > readVal);
	kernel.Terminate();
	DEBUG("SieveControllerNode %s end\n", GetName().c_str());
}

void SieveControllerNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&scnFactoryInstance);
}

void SieveControllerNode::Initialize(void) {
	ProducerNode::RegisterNodeType();
	FilterNode::RegisterNodeType();
	kernel.CreateNode(PRODUCER_NAME, SIEVE_PRODUCERNODE_TYPENAME, &param.numberBound, 0);
	SetupQueue(PRODUCER_NAME);
}

void SieveControllerNode::SetupQueue(const std::string& nodename) {
	queueCounter++;
	std::string queuename = toString(QUEUE_FORMAT, queueCounter);
	kernel.CreateQueue(queuename, param.queueTypeName,
			param.queueSize*sizeof(unsigned long),
			sizeof(unsigned long), 1);
	kernel.ConnectWriteEndpoint(queuename, nodename, OUT_PORT);
	kernel.ConnectReadEndpoint(queuename, GetName(), IN_PORT);
}

void SieveControllerNode::CreateFilter(const unsigned long prime) {
	std::string nodename = toString(FILTER_FORMAT, prime);
	kernel.CreateNode(nodename, SIEVE_FILTERNODE_TYPENAME, &prime, 0);
	std::string oldqname = toString(QUEUE_FORMAT, queueCounter);
	kernel.ConnectReadEndpoint(oldqname, nodename, IN_PORT);
	SetupQueue(nodename);
}


