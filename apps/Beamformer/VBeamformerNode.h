
#pragma once

#include "NodeBase.h"
#include <vector>

class VBeamformer;

#define VBEAMFORMERNODE_TYPENAME "VBeamformerNodeTypeName"

class VBeamformerNode : public CPN::NodeBase {
public:
    VBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
    ~VBeamformerNode();
private:
    void Process();

    std::string inport;
    std::vector<std::string> outports;
    VBeamformer *vbeam;
};
