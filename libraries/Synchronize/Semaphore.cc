/** \file
 */

#include "Semaphore.h"

void Sync::Semaphore::Wait(void) throw() {
	PthreadMutexProtected l(lock);
	while (0 == value) {
		cond.Wait(lock);
	}
	--value;
}

