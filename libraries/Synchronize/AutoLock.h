/** \file
 * Automatic locking on the stack.
 */

#ifndef AUTOLOCK_H
#define AUTOLOCK_H

#include "MutexBase.h"

class AutoLock : public MutexBase {
public:
	AutoLock(MutexBase& mutex_) : mutex(mutex_), count(0) {
		Wait();
	}
	~AutoLock() {
		while (!count)
			Release();
	}

	void Release() {
		--count;
		mutex.Release();
	}

	void Wait() {
		mutex.Wait();
		++count;
	}

private:
	MutexBase& mutex;
	unsigned long count;
};

#endif
