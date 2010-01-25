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
 * \brief Implementation for the NodeFactory registry.
 * \author John Bridgman
 */

#include "NodeFactory.h"
#include "PthreadMutex.h"
#include <map>

    
/// Lock to protect the factoryMap
static PthreadMutex lock;

typedef std::map< std::string, CPN::shared_ptr<CPN::NodeFactory> > FactoryMap;
/// mapping between the node type names and the factory implementation provided
/// by the modules.
static FactoryMap factorymap;

namespace CPN {

    NodeFactory::NodeFactory(const std::string &name_) : name(name_) {
    }

    NodeFactory::~NodeFactory() {
    }

}

CPN::shared_ptr<CPN::NodeFactory> CPNGetNodeFactory(const std::string &ntypename) {
    PthreadMutexProtected plock(lock);
    CPN::shared_ptr<CPN::NodeFactory> fact;
    FactoryMap::iterator item = factorymap.find(ntypename);
    if (item != factorymap.end()) {
        fact = item->second;
    }
    return fact;
}

void CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory> fact) {
    PthreadMutexProtected plock(lock);
    factorymap[fact->GetName()] = fact;
}

void CPNUnregisterNodeFactory(const std::string &ntypename) {
    PthreadMutexProtected plock(lock);
    factorymap.erase(ntypename);
}

