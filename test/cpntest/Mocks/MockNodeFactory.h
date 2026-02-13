
#ifndef MOCKNODEFACTORY_H
#define MOCKNODEFACTORY_H

#include "NodeFactory.h"

class MockNodeFactory : public CPN::NodeFactory {
public:
    MockNodeFactory(std::string name) : CPN::NodeFactory(name) {}

    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr);

private:
};

extern "C" {
    CPN::shared_ptr<CPN::NodeFactory> cpninitMockNode(void);
}

#endif

