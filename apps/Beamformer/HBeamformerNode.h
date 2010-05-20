
#pragma once

#include "NodeBase.h"

class HBeamformer;

class HBeamformerNode : public CPN::NodeBase {
public:
    HBeamformerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);
    ~HBeamformerNode();
private:
    void Process();

    std::string inport;
    std::string outport;
    HBeamformer *hbeam;
};

