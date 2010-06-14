
#pragma once

#include "NodeBase.h"
#include <string>
#include <vector>

#define BFINPUTNODE_TYPENAME "BFInputNodeTypeName"

class BFInputNode : public CPN::NodeBase {
public:
    BFInputNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
private:
    void Process();

    std::string outport;
    std::vector<std::string> infiles;
    unsigned repetitions;
};

