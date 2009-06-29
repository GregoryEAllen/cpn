/** \file
 */

#include "ReentrantLock.h"

void Sync::ReentrantLock::Wait(void) throw() {
	PthreadMutexProtected l(lock);
	if ( count && ( pthread_equal(owner, pthread_self()) ) ) {
		++count;
		return;
	}
	while (count) {
		cond.Wait(lock);
	}
	++count;
	owner = pthread_self();
}

