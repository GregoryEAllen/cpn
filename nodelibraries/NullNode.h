
#pragma once

#include "NodeBase.h"

#define NULLNODE_TYPENAME "NullNode"

class NullNode : public CPN::NodeBase {
public:
    NullNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
private:
    void Process();
    std::string inport;
};
