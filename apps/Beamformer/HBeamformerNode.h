
#pragma once

#include "NodeBase.h"

class HBeamformer;

/*
 * This node expects a json formatted string of paramters in the string parameter
 * section that conforms to the following template:
 *
 * {
 *  "inport"    : name of the input port,
 *  "outport"   : name of the output port,
 *  "estimate"  : true|false (optional, default false)
 *  "length"    : unsigned,
 *  "numStaves" : unsigned,
 *  "numBeams"  : unsigned,
 *  "staveIndexes" : [ ... ]
 * }
 * It is expected that the arg parameter will contain the coeffs and then
 * the replicas, in that order.
 */

#define HBEAMFORMERNODE_TYPENAME "HBeamformerNode"

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

