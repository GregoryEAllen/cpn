/** \file
 * Base idea for a generalized mutex.
 */

#ifndef SYNC_WAITABLE_H
#define SYNC_WAITABLE_H

namespace Sync {
	/**
	 * Generic representation of an object that manages some
	 * resource that can be released and waited upon.
	 * Waiting consumes or decreases the resource
	 * and releasing increases or puts back the resource.
	 * There may different kinds of limits on consumption
	 * and releasing.
	 */
	class Waitable {
	public:
		virtual ~Waitable() {};
		virtual void Release(void) throw() = 0;
		virtual void Wait(void) throw() = 0;
	};
}
#endif
