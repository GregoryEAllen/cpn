/** \file
 * Base idea for a generalized mutex.
 */

#ifndef MUTEXBASE_H
#define MUTEXBASE_H

namespace Sync {
	/**
	 * Generic representation of an object that manages some
	 * resource that can be released and waited upon.
	 * Waiting consumes or decreases the resource
	 * and releasing increases or puts back the resource.
	 * There may different kinds of limits on consumption
	 * and releasing.
	 */
	class MutexBase {
	public:
		virtual ~MutexBase() {};
		virtual void Release(void) throw() = 0;
		virtual void Wait(void) throw() = 0;
	};
}
#endif
