/** \file
 * \brief FilterNode implementation.
 */

#include "FilterNode.h"
#include "Kernel.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "NodeFactory.h"

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

class FilterFactory : public CPN::NodeFactory {
public:
	FilterFactory() : CPN::NodeFactory(SIEVE_FILTERNODE_TYPENAME) {}
	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			const void* const arg, const CPN::ulong argsize) {
		FilterNode::Param* p = (FilterNode::Param*)arg;
		return new FilterNode(ker, attr, p->filterval, p->threshold);
	}
	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};

static FilterFactory filterFactoryInstance;

void FilterNode::Process(void) {
	DEBUG("FilterNode %s start\n", GetName().c_str());
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
	DEBUG("FilterNode %s end\n", GetName().c_str());
}

void FilterNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&filterFactoryInstance);
}
