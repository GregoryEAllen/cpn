/** \file
 * Declaration of the queue status variable.
 */

#ifndef CPN_QUEUESTATUS_H
#define CPN_QUEUESTATUS_H

namespace CPN {
	struct QueueStatus {
		enum Status_t {
			READY,
			BLOCKED,
			QUERY,
			DETACHED,
			SHUTDOWN,
			TRANSFER
		};
		QueueStatus(Status_t status_) : status(status_) {}
		bool operator==(const QueueStatus& o) { return status == o.status; }
		bool operator!=(const QueueStatus& o) { return status != o.status; }
		Status_t status;
	};
}
#endif

