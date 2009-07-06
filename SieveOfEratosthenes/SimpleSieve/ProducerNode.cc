/** \file
 * \brief ProducerNode implementation
 */

#include "ProducerNode.h"
#include "NodeFactory.h"
#include "QueueWriterAdapter.h"
#include "Kernel.h"

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

class ProducerFactory : public CPN::NodeFactory {
public:	
	ProducerFactory() : CPN::NodeFactory(SIEVE_PRODUCERNODE_TYPENAME) {}

	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr &attr,
		       	const void* const arg, const CPN::ulong argsize) {
		SieveControllerNode::Param* p = (SieveControllerNode::Param*)arg;
		return new ProducerNode(ker, attr, *p);
	}

	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};

static ProducerFactory producerFactoryInstance;

void ProducerNode::Process(void) {
	DEBUG("ProducerNode %s start\n", GetName().c_str());
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(GetName(), "y");
	const unsigned long cutoff = param.numberBound;
	unsigned long counter = 2;
	while (cutoff == 0 || counter < cutoff) {
		out.Enqueue(&counter, 1);
		++counter;
	}
	// Ending marker
	counter = 0;
	out.Enqueue(&counter, 1);
	DEBUG("ProducerNode %s end\n", GetName().c_str());
}

void ProducerNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&producerFactoryInstance);
}


