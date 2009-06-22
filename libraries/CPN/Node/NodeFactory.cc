/** \file
 */

#include "NodeFactory.h"


static PthreadMutex lock;

static ::std::map< ::std::string, ::CPN::NodeFactory* > factoryMap;

CPN::NodeFactory::NodeFactory(const ::std::string &name_) : name(name_) {
	PthreadMutexProtected plock(lock);
	factoryMap[name] = this;
}


CPN::NodeFactory::~NodeFactory() {
	PthreadMutexProtected plock(lock);
	factoryMap.erase(name);
}

CPN::NodeFactory* CPN::NodeFactory::GetFactory(const ::std::string &name) {
	PthreadMutexProtected plock(lock);
	return factoryMap[name];
}

