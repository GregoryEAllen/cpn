/** \file
 * Declaration of the queue status variable.
 */

#ifndef CPN_QUEUESTATUS_H
#define CPN_QUEUESTATUS_H

namespace CPN {
	struct QueueStatus {
		/**
		 * The states of the queue endpoints.
		 *
		 * The following are the legal transitions.
		 * <code>
		 * READY => DETACHED, QUERY, SHUTDOWN;
		 * BLOCKED => READY, DETACHED, SHUTDOWN;
		 * QUERY => BLOCKED, READY, TRANSFER, SHUTDOWN;
		 * DETACHED => READY, SHUTDOWN;
		 * TRANSFER => READY, SHUTDOWN;
		 * SHUTDOWN => Cannot transition out of shutdown;
		 * D4RTRANSMIT Unimplemented at this time;
		 * </code>
		 */
		enum Status_t {
			/// The endpoint is ready for operation.
			READY,
			/// The endpoint is waiting for the queue to become ready.
			BLOCKED,
			/// The endpoint is currently querying the queue for an operation.
			QUERY,
			/// The endpoint does not have a queue associated with it.
			DETACHED,
			/// Force shutdown on all operations.
			SHUTDOWN,
			/// The endpoint is currently in the middle of an operation.
			TRANSFER,
			/// The queue has D4R information ready for this endpoint.
			D4RTRANSMIT
		};
		QueueStatus(Status_t status_) : status(status_) {}
		bool operator==(const QueueStatus& o) { return status == o.status; }
		bool operator!=(const QueueStatus& o) { return status != o.status; }
		Status_t status;
		// D4R info
	};
}
#endif

