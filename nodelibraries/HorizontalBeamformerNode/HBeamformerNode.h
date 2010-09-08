//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 * A CPN node that encapsulates a horizontal beamformer.
 */
#ifndef HBEAMFORMERNODE_H
#define HBEAMFORMERNODE_H
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
    int half;
};
#endif
