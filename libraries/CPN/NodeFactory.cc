/** \file
 * Implementation for the NodeFactory registry.
 */

#include "NodeFactory.h"
#include "PthreadMutex.h"
#include <map>

/// Lock to protect the factoryMap
static PthreadMutex lock;

/// mapping between the node type names and the factory implementation provided
/// by the modules.
static ::std::map< ::std::string, ::CPN::NodeFactory* > factoryMap;

CPN::NodeFactory::NodeFactory(const ::std::string &name_) : name(name_) {
	PthreadMutexProtected plock(lock);
	factoryMap[name] = this;
}

CPN::NodeFactory::~NodeFactory() {
	PthreadMutexProtected plock(lock);
	factoryMap.erase(name);
}

CPN::NodeFactory* CPN::NodeFactory::GetFactory(const ::std::string &ntypename) {
	PthreadMutexProtected plock(lock);
	return factoryMap[ntypename];
}

