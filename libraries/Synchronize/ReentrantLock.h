/** \file
 * A reentrant lock implementation.
 */

#ifndef REENTRANTLOCK_H
#define REENTRANTLOCK_H

#include "MutexBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <pthread.h>

namespace Sync {
	/**
	 * A reentrant lock.
	 */
	class ReentrantLock : public MutexBase {
	public:
		ReentrantLock() throw() : count(0) {}

		void Release(void) throw() {
			PthreadMutexProtected l(lock);
			--count;
			cond.Signal();
		}

		void Wait(void) throw();

	private:
		PthreadMutex lock;
		PthreadCondition cond;
		unsigned long count;
		pthread_t owner;
	};
}
#endif
