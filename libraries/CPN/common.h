/** \file
 * \brief Common definitions to the entire CPN library.
 */

#ifndef CPN_COMMON_H
#define CPN_COMMON_H
namespace CPN {
	/**
	 * Each Host must have a unique ID.
	 */
	typedef unsigned long HostID;

	/**
	 * Each node must have a unique ID os that we can
	 * recognize them acorss threads, processes, and hosts.
	 */
	typedef unsigned long NodeID;

	/**
	 * Each queue must have a unique identifier to so
	 * that the queue can be recognized across threads,
	 * processes, and hosts.
	 */
	typedef unsigned long QueueID;

}
#endif
