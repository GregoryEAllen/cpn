/** \file
 */

#include "Event.h"


void Sync::Event::Wait(void) throw() {
	PthreadMutexProtected l(lock);
	while (!signaled) {
		cond.Wait(lock);
	}
	if (automatic) {
		signaled = false;
	}
}

