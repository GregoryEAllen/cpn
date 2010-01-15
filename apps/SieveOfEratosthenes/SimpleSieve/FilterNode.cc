/** \file
 * \brief FilterNode implementation.
 */

#include "FilterNode.h"
#include "Kernel.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "NodeFactory.h"
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

class FilterFactory : public CPN::NodeFactory {
public:
	FilterFactory() : CPN::NodeFactory(SIEVE_FILTERNODE_TYPENAME) {}
    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel& ker, const CPN::NodeAttr& attr) {
        ASSERT(attr.GetArg().GetBuffer());
        ASSERT(attr.GetArg().GetSize() == sizeof(FilterNode::Param));
		FilterNode::Param* p = (FilterNode::Param*)attr.GetArg().GetBuffer();
		return CPN::shared_ptr<CPN::NodeBase>(new FilterNode(ker, attr, p->filterval, p->threshold));
	}
};

static FilterFactory filterFactoryInstance;

void FilterNode::Process(void) {
	DEBUG("FilterNode %s start\n", GetName().c_str());
	CPN::QueueWriterAdapter<unsigned long> out = GetWriter("prodport");
	CPN::QueueReaderAdapter<unsigned long> in = GetReader("x");
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

void FilterNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory>(new FilterFactory));
}
