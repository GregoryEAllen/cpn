
#pragma once

#include "NodeBase.h"
#include <string>
#include <vector>

#define JOINNODE_TYPENAME "JoinNode"

class JoinNode : public CPN::NodeBase {
public:
    JoinNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
private:
    void Process();
    std::vector<std::string> inports;
    std::string outport;
    unsigned size;
    unsigned overlap;
};

