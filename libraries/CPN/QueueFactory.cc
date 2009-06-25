/** \file
 * Implementation for the QueueFactory registry.
 */

#include "QueueFactory.h"
#include "PthreadMutex.h"
#include <map>
#include <cstdio>

/// Lock to protect queueMap from concurrent access
static PthreadMutex lock;

/// The mapping between queue type names and the
/// factories for them.
static ::std::map< ::std::string, CPN::QueueFactory* > queueMap;

CPN::QueueFactory::QueueFactory(const ::std::string &name_) : name(name_) {
}

CPN::QueueFactory::~QueueFactory() {
	CPNUnregisterQueueFactory(name);
}

CPN::QueueFactory* CPNGetQueueFactory(const ::std::string& qtypename) {
	PthreadMutexProtected plock(lock);
	return queueMap[qtypename];
}


void CPNRegisterQueueFactory(CPN::QueueFactory* fact) {
	PthreadMutexProtected plock(lock);
	queueMap[fact->GetName()] = fact;
}

void CPNUnregisterQueueFactory(const ::std::string& qtypename) {
	PthreadMutexProtected plock(lock);
	queueMap.erase(qtypename);
}

