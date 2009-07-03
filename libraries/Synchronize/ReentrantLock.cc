/** \file
 */

#include "ReentrantLock.h"

void Sync::ReentrantLock::Wait(void) throw() {
	PthreadMutexProtected l(lock);
	if ( count != 0 && ( pthread_equal(owner, pthread_self()) ) ) {
		++count;
	} else {
		while (count != 0) { cond.Wait(lock); }
		++count;
		owner = pthread_self();
	}
}

void Sync::ReentrantLock::Wait(PthreadCondition& c) throw() {
	unsigned long oldcount = count;
	pthread_t oldowner = owner;
	count = 0;
	cond.Signal();
	c.Wait(lock);
	while (count != 0) { cond.Wait(lock); }
	count = oldcount;
	owner = oldowner;
}

