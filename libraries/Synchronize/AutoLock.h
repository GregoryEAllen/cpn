/** \file
 * Automatic locking on the stack.
 */

#ifndef SYNC_AUTOLOCK_H
#define SYNC_AUTOLOCK_H

#include "Waitable.h"

namespace Sync {
	class AutoLock : public Waitable {
	public:
		AutoLock(Waitable& mutex_) throw() : mutex(mutex_), count(0) {
			Wait();
		}
		~AutoLock() throw() {
			while (count != 0)
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
		Waitable& mutex;
		unsigned long count;
	};
}
#endif
