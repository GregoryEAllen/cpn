
#include "MockNodeFactory.h"
#include "MockNode.h"
#include <cassert>


CPN::shared_ptr<CPN::NodeBase> MockNodeFactory::Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
	return CPN::shared_ptr<CPN::NodeBase>(new MockNode(ker, attr));
}


