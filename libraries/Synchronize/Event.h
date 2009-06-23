/** \file
 * \brief A very simple "event" like synchronization
 * object. Very similar in some respects to the
 * windows Event object.
 */

#ifndef EVENT_H
#define EVENT_H

#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "MutexBase.h"

/**
 * A very simple "event" similar in some ways to the
 * windows Event object.
 *
 * An Event can be in one of two states. Signaled and
 * unsignaled. A call to Wait will cause the thread
 * to suspend until the Event becomes signaled.
 * If the event is automatic then after a successful
 * Wait the event will automatically become non signaled.
 * If the Event is not automatic then all threads Waiting
 * on this event will succeed and all future Waits will
 * succeed immediately until Reset is called.
 * 
 */
class Event : public MutexBase {
public:
	/**
	 * Default constructor creates a manual reset
	 * event that is not signaled.
	 */
	Event() : automatic(false), signaled(false) {}

	/**
	 * Create an event.
	 * \param automatic true will be automatic, false will be manual
	 * \param signaled true will start out signaled, false will start out not signaled
	 */
	Event(bool automatic, bool signaled = false) :
	       	automatic(automatic), signaled(signaled) {}

	/**
	 * Signal this event.
	 */
	void Signal(void) {
		PthreadMutexProtected l(lock);
		signaled = true;
		cond.Broadcast();
	}

	void Release(void) { Signal(); }

	/**
	 * Reset this event.
	 */
	void Reset(void) {
		PthreadMutexProtected l(lock);
		signaled = false;
	}

	/**
	 * Wait for this event to become signaled.
	 * If it is already signaled then return immediately.
	 * This method will reset the event if the event
	 * is an automatic event.
	 */
	void Wait(void) {
		PthreadMutexProtected l(lock);
		while (!signaled) {
			cond.Wait(lock);
		}
		if (automatic) {
			signaled = false;
		}
	}

private:
	const bool automatic;
	bool signaled;
	PthreadMutex lock;
	PthreadCondition cond;
};

#endif

