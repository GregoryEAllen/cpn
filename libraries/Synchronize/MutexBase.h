/** \file
 * Base idea for a generalized mutex.
 */

#ifndef MUTEXBASE_H
#define MUTEXBASE_H

class MutexBase {
public:
	virtual void Release(void) = 0;
	virtual void Wait(void) = 0;
};

#endif
