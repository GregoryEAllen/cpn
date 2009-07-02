/** \file
 * \brief ProducerNode implementation
 */

#include "ProducerNode.h"
#include "NodeFactory.h"
#include "QueueWriterAdapter.h"
#include "Kernel.h"

class ProducerFactory : public CPN::NodeFactory {
public:	
	ProducerFactory() : CPN::NodeFactory(SIEVE_PRODUCERNODE_TYPENAME) {}

	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr &attr,
		       	const void* const arg, const CPN::ulong argsize) {
		return new ProducerNode(ker, attr, *((unsigned long*)arg));
	}

	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};

static ProducerFactory producerFactoryInstance;

void ProducerNode::Process(void) {
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(GetName(), "y");
	unsigned long counter = 2;
	while (cutoff == 0 || counter < cutoff) {
		out.Enqueue(&counter, 1);
		++counter;
	}
	counter = 0;
	out.Enqueue(&counter, 1);
}

void ProducerNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&producerFactoryInstance);
}


