
#pragma once

#include "NodeBase.h"
#include <string>
#include <vector>

#define FORKJOINNODE_TYPENAME "ForkJoinNodeType"

class ForkJoinNode : public CPN::NodeBase {
public:
    ForkJoinNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
private:
    void Process();
    std::vector<std::string> inports;
    std::vector<std::string> outports;
    unsigned size;
    unsigned overlap;
};

