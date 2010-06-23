
#pragma once

#include "NodeBase.h"
#include <vector>

class FanVBeamformer;

#define FANVBEAMFORMERNODE_TYPENAME "FanVBeamformerNode"

class FanVBeamformerNode : public CPN::NodeBase {
public:
    FanVBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
    ~FanVBeamformerNode();
private:
    void Process();

    std::string inport;
    std::vector<std::string> outports;
    unsigned blocksize;
    FanVBeamformer *vbeam;
};
