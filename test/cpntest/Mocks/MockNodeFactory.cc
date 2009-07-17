
#include "MockNodeFactory.h"
#include "MockNode.h"
#include <cassert>

MockNodeFactory MockNodeFactory::instance("MockNode");

CPN::NodeBase* MockNodeFactory::Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
		const void* const arg, const CPN::ulong argsize) {
	assert(argsize == sizeof(MockNode::Mode_t));
	MockNode::Mode_t mode = *((const MockNode::Mode_t*)arg);
	return new MockNode(ker, attr, mode);
}
CPN::NodeBase* MockNodeFactory::Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
	return new MockNode(ker, attr);
}
CPN::NodeBase* MockNodeFactory::Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
		const std::string &param) {
	return new MockNode(ker, attr, param);
}

void MockNodeFactory::Destroy(CPN::NodeBase* node) {
	delete node;
}

