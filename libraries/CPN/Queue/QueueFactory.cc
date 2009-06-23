/** \file
 * Implementation for the QueueFactory registry.
 */

#include "QueueFactory.h"
#include "PthreadMutex.h"
#include <map>

/// Lock to protect queueMap from concurrent access
static PthreadMutex lock;

/// The mapping between queue type names and the
/// factories for them.
static ::std::map< ::std::string, CPN::QueueFactory* > queueMap;

CPN::QueueFactory::QueueFactory(const ::std::string name_) : name(name_) {
	PthreadMutexProtected plock(lock);
	queueMap[name] = this;
}

CPN::QueueFactory::~QueueFactory() {
	PthreadMutexProtected plock(lock);
	queueMap.erase(name);
}

CPN::QueueFactory* CPN::QueueFactory::GetFactory(const ::std::string& qtypename) {
	PthreadMutexProtected plock(lock);
	return queueMap[qtypename];
}


