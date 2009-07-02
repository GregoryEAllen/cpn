/** \file
 * \brief FilterNode implementation.
 */

#include "FilterNode.h"
#include "Kernel.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "NodeFactory.h"

#include <cstdio>

class FilterFactory : public CPN::NodeFactory {
public:
	FilterFactory() : CPN::NodeFactory(SIEVE_FILTERNODE_TYPENAME) {}
	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			const void* const arg, const CPN::ulong argsize) {
		return new FilterNode(ker, attr, *((unsigned long*)arg));
	}
	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};

static FilterFactory filterFactoryInstance;

void FilterNode::Process(void) {
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(GetName(), "y");
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(GetName(), "x");
	unsigned long nextFilter = filterval;
	unsigned long readVal = 0;
	do {
		in.Dequeue(&readVal, 1);
		if (readVal == nextFilter) {
			nextFilter += filterval;
		} else {
			if (readVal > nextFilter) {
				nextFilter += filterval;
			}
			out.Enqueue(&readVal, 1);
		}
	} while (readVal != 0);
}

void FilterNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&filterFactoryInstance);
}
