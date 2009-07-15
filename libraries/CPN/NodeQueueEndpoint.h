/** \file
 */

#ifndef CPN_NODEQUEUEENDPOINT_H
#define CPN_NODEQUEUEENDPOINT_H

#include "QueueStatus.h"
#include "StatusHandler.h"
#include "QueueInfo.h"
#include "ReentrantLock.h"
#include <string>

namespace CPN {
	class QueueInfo;
	class NodeInfo;
	class QueueBase;

	/**
	 * This class is an abstraction of the common functions
	 * of both the QueueReader and QueueWriter. It provides
	 * functionality for blocking and unblocking when the queue
	 * if full or empty.
	 *
	 */
	class NodeQueueEndpoint {
	public:

		NodeQueueEndpoint(NodeInfo* nodeinfo_, const std::string &portname_);

		virtual ~NodeQueueEndpoint();

		/**
		 * Set the queue that the reader should use to read with.
		 * It is illegal to call this function when this endpoint
		 * is already connected to a queue. ClearQueueInfo must
		 * be called first.
		 * \param queueinfo_ the QueueInfo object that holds the queue
		 */
		void SetQueueInfo(QueueInfo* queueinfo_);

		/**
		 * Remove the queue from this reader.
		 * \param checkdeath whether this clearing indicates that
		 * the queue is now ready for cleanup.
		 */
		void ClearQueueInfo(bool checkdeath);

		/**
		 * \return a pointer to the status handler for this reader.
		 */
		Sync::StatusHandler<QueueStatus>* GetStatusHandler(void) { return &status; }

		/**
		 * \return the QueueInfo object registered with us or 0
		 */
		QueueInfo* GetQueueInfo(void);

		/**
		 * Sets the endpoint to terminate. Next call to a queue
		 * function will cause the node to stop.
		 */
		void Terminate(void);

		/**
		 * \return a pointer to the NodeInfo we are associated with
		 */
		NodeInfo* GetNodeInfo(void) { return nodeinfo; }

		/**
		 * \return our name
		 */
		const std::string &GetPortName(void) const { return portname; }

	protected:
		/**
		 * Pure virtual function to be implemented in
		 * derived classes to call the correct endpoint function
		 * in QueueInfo.
		 * \param qinfo the QueueInfo object to call QueueInfo::SetWriter
		 * or QueueInfo::SetReader upon.
		 */
		virtual void SetQueueInfoEndpoint(QueueInfo* qinfo) = 0;
		/**
		 * Pure virtual function to be implemented in
		 * derived classes to call the correct endpoint function
		 * in QueueInfo.
		 * \param qinfo the QueueInfo object to call QueueInfo::ClearWriter
		 * or QueueInfo::ClearReader upon.
		 * \param checkdeath the parameter for QueueInfo::ClearWriter or
		 * QueueInfo::ClearReader
		 */
		virtual void ClearQueueInfoEndpoint(QueueInfo* qinfo, bool checkdeath) = 0;

		/**
		 * This function blocks if no QueueInfo has been set
		 * or returns the QueueBase associated with this endpoint.
		 * \note Caller must be holding lock.
		 * \return The QueueBase associated with ths endpoint
		 * \throw KernelShutdownException
		 */
		QueueBase* CheckQueue(void) const;
		/**
		 * Start a enqueue or dequeue operation.
		 * Calls CheckQueue.
		 * \note Caller must be holding lock.
		 * \return the QueueBase to do the actual calls on
		 * \throw KernelShutdownException
		 */
		QueueBase* StartOperation(void);
		/**
		 * Block this thread after a failed enqueue or dequeue
		 * StartOperation must have been called before the enqueue or dequeue
		 * or undefined behavior will result.
		 * \note Caller must be holding lock
		 */
		void Block(void);
		/**
		 * Sets our state correctly for a continued enqueue or dequeue.
		 * This functionality is required for the GetEnqueuePtr/GetDequeuePtr
		 * and Enqueue/Dequeue functions.
		 * \note Caller must be holding lock
		 */
		void ContinueOperation(void);
		/**
		 * Mark the operation as complete and return us to the READY state.
		 * This should be called at the end of Enqueue or Dequeue.
		 * \note Caller must be holding lock
		 */
		void CompleteOperation(void);

		NodeInfo* nodeinfo;
		const std::string portname;
		mutable Sync::ReentrantLock lock;
		QueueInfo* queueinfo;
		Sync::StatusHandler<QueueStatus> status;

	};
}

#endif

