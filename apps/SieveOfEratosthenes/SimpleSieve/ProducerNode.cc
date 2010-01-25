/** \file
 * \brief ProducerNode implementation
 */

#include "ProducerNode.h"
#include "NodeFactory.h"
#include "QueueWriterAdapter.h"
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

class ProducerFactory : public CPN::NodeFactory {
public:	
	ProducerFactory() : CPN::NodeFactory(SIEVE_PRODUCERNODE_TYPENAME) {}

    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel& ker, const CPN::NodeAttr &attr) {
        ASSERT(attr.GetArg().GetBuffer());
        ASSERT(attr.GetArg().GetSize() == sizeof(SieveControllerNode::Param));
		SieveControllerNode::Param* p = (SieveControllerNode::Param*)attr.GetArg().GetBuffer();
		return CPN::shared_ptr<CPN::NodeBase>(new ProducerNode(ker, attr, *p));
	}
};


void ProducerNode::Process(void) {
	DEBUG("ProducerNode %s start\n", GetName().c_str());
	CPN::QueueWriterAdapter<unsigned long> out = GetWriter("y");
	const unsigned long cutoff = param.numberBound;
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

extern "C" {
    CPN::shared_ptr<CPN::NodeFactory> cpninitSieveProducerNodeTypeName(void);
}

CPN::shared_ptr<CPN::NodeFactory> cpninitSieveProducerNodeTypeName(void) {
	return (CPN::shared_ptr<CPN::NodeFactory>(new ProducerFactory));
}


