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

	template<class Status_t>
	class StatusHandler;
	/**
	 * A reentrant lock.
	 */
	class ReentrantLock : public MutexBase {
	public:
		ReentrantLock() throw() : count(0) {}
		~ReentrantLock() throw() {}

		void Release(void) throw() {
			PthreadMutexProtected l(lock);
			--count;
			if (count == 0) cond.Signal();
		}

		void Wait(void) throw();

	private:
		void Wait(PthreadCondition& c) throw();
		PthreadMutex lock;
		PthreadCondition cond;
		unsigned long count;
		pthread_t owner;

		template<class T> friend class StatusHandler;
	};
}
#endif
