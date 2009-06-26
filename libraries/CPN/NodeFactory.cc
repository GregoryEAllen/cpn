/** \file
 * Implementation for the NodeFactory registry.
 */

#include "NodeFactory.h"
#include "PthreadMutex.h"
#include <map>
#include <cstdio>

/// Lock to protect the factoryMap
static PthreadMutex lock;

/// mapping between the node type names and the factory implementation provided
/// by the modules.
static ::std::map< ::std::string, ::CPN::NodeFactory* > factoryMap;

CPN::NodeFactory::NodeFactory(const ::std::string &name_) : name(name_) {
}

CPN::NodeFactory::~NodeFactory() {
}

CPN::NodeFactory* CPNGetNodeFactory(const ::std::string &ntypename) {
	PthreadMutexProtected plock(lock);
	return factoryMap[ntypename];
}

void CPNRegisterNodeFactory(CPN::NodeFactory* fact) {
	PthreadMutexProtected plock(lock);
	factoryMap[fact->GetName()] = fact;
}

void CPNUnregisterNodeFactory(const ::std::string &ntypename) {
	PthreadMutexProtected plock(lock);
	factoryMap.erase(ntypename);
}

