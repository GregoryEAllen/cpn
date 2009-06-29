/** \file
 * \brief Provide a very simple semaphore class.
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "MutexBase.h"

namespace Sync {
	/**
	 * A very simple semaphore implemented using a mutex
	 * and a condition variable.
	 * No timed wait or try wait functionality provided.
	 *
	 * This implementation allows one to do bulk post to
	 * the semaphore.
	 */
	class Semaphore : public MutexBase {
	public:
		typedef unsigned long ulong;
		Semaphore() throw() : value(0) { }

		Semaphore(ulong value) throw() : value(value) { }

		/**
		 * Post to num to the semaphore.
		 *
		 * \param num how many to add to the semaphore (default 1)
		 */
		void Post(ulong num = 1) throw() {
			PthreadMutexProtected l(lock);
			value += num;
			// broadcast because we might have
			// more than one thread waiting
			cond.Broadcast();
		}

		void Release(void) throw() { Post(); }

		/**
		 * Wait for the semaphore to be greater than 0
		 * and then decrement by one.
		 */
		void Wait(void) throw();

		/**
		 * \return the current value of the semaphore.
		 */
		ulong GetValue(void) throw() {
			PthreadMutexProtected l(lock);
			return value;
		}
	private:
		ulong value;
		PthreadMutex lock;
		PthreadCondition cond;
	};
}
#endif

