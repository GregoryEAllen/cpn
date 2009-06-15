/** \file
 * \brief Definition for the queue messages used
 * in the D4R algorithm.
 */

#ifndef CPN_QUEUE_MESSAGEBASE_H
#define CPN_QUEUE_MESSAGEBASE_H
namespace CPN::Queue {

	/**
	 * The base class for all queue messages.
	 * This class and all derived classes must 
	 * have trivial constructors and distructors
	 * and all member variables have trivial constructors
	 * and destructors. This class and all derived classes must
	 * have no reference types.
	 *
	 * I.e. a memcpy of this class must be valid.
	 */
	class MessageBase {
	public:
		MessageBase(int mid) ID(mid) {}
		int getID(void) const { return ID; }
	private:
		const int ID;
	};
}
#endif
