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
 * The JoinNode reads from the inputs in a round robbin fashion and
 * then writes to the output, optionally discarding some portion read.
 */
#ifndef JOINNODE_H
#define JOINNODE_H
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
#endif
