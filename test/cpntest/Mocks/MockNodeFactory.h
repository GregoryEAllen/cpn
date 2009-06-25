
#ifndef MOCKNODEFACTORY_H
#define MOCKNODEFACTORY_H

#include "NodeFactory.h"

class MockNodeFactory : public CPN::NodeFactory {
public:
	MockNodeFactory(std::string name) : CPN::NodeFactory(name) {}

	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
		       	const void* const arg, const CPN::ulong argsize);

	void Destroy(CPN::NodeBase* node);

	static CPN::NodeFactory* GetInstance() { return &instance; }
private:
	static MockNodeFactory instance;
};

#endif

