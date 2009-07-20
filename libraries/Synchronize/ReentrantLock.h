/** \file
 * A reentrant lock implementation.
 */

#ifndef SYNC_REENTRANTLOCK_H
#define SYNC_REENTRANTLOCK_H

#include "Waitable.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <pthread.h>

namespace Sync {

	template<class Status_t>
	class StatusHandler;
	/**
	 * A reentrant lock.
	 */
	class ReentrantLock : public Waitable {
	public:
		ReentrantLock() throw() : count(0) {}
		~ReentrantLock() throw() {}

		void Release(void) throw() {
			PthreadMutexProtected l(lock);
			--count;
			if (count == 0) cond.Signal();
		}

		void Wait(void) throw();

		/**
		 * Use only for testing purposes and asserts.
		 * \return true if the calling thread has the lock.
		 */
		bool HaveLock(void) throw();

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
