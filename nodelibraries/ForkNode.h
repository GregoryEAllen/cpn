
#pragma once

#include "NodeBase.h"
#include <string>
#include <vector>

#define FORKNODE_TYPENAME "ForkNode"

class ForkNode : public CPN::NodeBase {
public:
    ForkNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
private:
    void Process();
    std::string inport;
    std::vector<std::string> outports;
    unsigned size;
    unsigned overlap;
};

