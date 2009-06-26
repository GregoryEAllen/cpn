/** \file
 * Automatic locking on the stack.
 */

#ifndef AUTOLOCK_H
#define AUTOLOCK_H

#include "MutexBase.h"

namespace Sync {
	class AutoLock : public MutexBase {
	public:
		AutoLock(MutexBase& mutex_) throw() : mutex(mutex_), count(0) {
			Wait();
		}
		~AutoLock() throw() {
			while (!count)
				Release();
		}

		void Release() throw() {
			--count;
			mutex.Release();
		}

		void Wait() throw() {
			mutex.Wait();
			++count;
		}

	private:
		MutexBase& mutex;
		unsigned long count;
	};
}
#endif
