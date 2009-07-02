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

