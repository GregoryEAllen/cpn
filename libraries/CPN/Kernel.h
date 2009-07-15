/** \file
 * \brief Definition for the kernel object.
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H

#include "KernelAttr.h"
#include "NodeFactory.h"
#include "QueueFactory.h"
#include "StatusHandler.h"
#include "ReentrantLock.h"
#include "AutoLock.h"
#include <string>
#include <map>
#include <deque>

namespace CPN {

	class QueueReader;
	class QueueWriter;
	class NodeInfo;
	class QueueInfo;

	/**
	 * \brief The Kernel declaration.
	 *
	 * The purpose of the kernel object is to keep track
	 * of all the queues and nodes on a particular host,
	 * ensure that they are instantiated and destroyed
	 * correctly and to provide a unified interface to
	 * the user of the process network.
	 */
	class Kernel {
	public:
		enum Status_t { READY, RUNNING, TERMINATING, STOPPED };

		/**
		 * Construct a new kernel object with the given name and id.
		 */
		Kernel(const KernelAttr &kattr);
		~Kernel();

		/**
		 * Start the nodes in the process network.
		 */
		void Start(void);

		/**
		 * Wait for all nodes to terminate and does cleanup.
		 * Note that no resources will be cleaned up until
		 * destruction if this function is never called.
		 */
		void Wait(void);

		/**
		 * Force all running nodes to terminate.
		 * Returns immediately use Wait if one wishes
		 * to wait for the nodes to terminate.
		 * Has no effect if not started.
		 */
		void Terminate(void);

		/**
		 * \return the current status
		 */
		Status_t GetStatus(void) const { return statusHandler.Get(); }

		/**
		 * Compare the current status to oldStatus and wait
		 * for the status to be different.
		 *
		 * \param oldStatus the status to compare
		 * \return the new status
		 */
		Status_t CompareStatusAndWait(Status_t oldStatus) const {
			Sync::AutoLock plock(lock); 
			return statusHandler.CompareAndWait(oldStatus); }

		/**
		 * Create a new node.
		 * \param nodename the name for the new node
		 * \param nodetype the type name of the new node
		 * \param arg an optional void pointer to arguments for the node
		 * may be 0
		 * \param argsize an optional size for the arg, this is
		 * implementation dependent see documentation for the node
		 * type you are creating.
		 * \throws KernelShutdownException if Wait completed or Terminate
		 * has been called.
		 * \throws std::invalid_argument if nodename already exists
		 */
		void CreateNode(const std::string &nodename,
				const std::string &nodetype,
				const void* const arg,
				const ulong argsize);

		/**
		 * Create a new queue.
		 * \param queuename the name for the new queue
		 * \param queuetype the type name for the queue
		 * \param queueLength the initial length for the queue
		 * \param maxThreshold the initial maximum threadshold for the
		 * new queue
		 * \param numChannels the number of channels for the queue
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if queuename already exists.
		 */
		void CreateQueue(const std::string &queuename,
				const std::string &queuetype,
				const ulong queueLength,
				const ulong maxThreshold,
				const ulong numChannels);

		/**
		 * Connects the write endpoint of the given queue to the
		 * given port on the given node.
		 * It is an illegal operation to try to connect a queue
		 * to a port that already has a queue connected to it.
		 * \param qname the name of the queue to connect
		 * \param nodename the name of the node to connect
		 * \param portname the name of the writer port on the node
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if the queue or node
		 * do not exist.
		 */
		void ConnectWriteEndpoint(const std::string &qname,
				const std::string &nodename,
				const std::string &portname);

		/**
		 * Remove the writer associated with the given endpoint.
		 * If as a result of this call the queue is not connected
		 * on both ends then it will be deleted.
		 * \param nodename the name of the node
		 * \param portname the name of the port
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if nodename does not exist.
		 */
		void RemoveWriteEndpoint(const std::string &nodename,
				const std::string &portname);
		/**
		 * Connects the reader endpoint of the given queue to the
		 * given port on the given node.
		 * It is an illegal operation to try to connect a queue
		 * to a port that already has a queue connected to it.
		 * \param qname the name of the queue to connect
		 * \param nodename the name of the node to connect
		 * \param portname the name of the reader port on the node
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if the queue or node
		 * do not exist.
		 */
		void ConnectReadEndpoint(const std::string &qname,
				const std::string &nodename,
				const std::string &portname);

		/**
		 * Remove the reader associated with the given endpoint.
		 * If as a result of this call the queue is not connected
		 * on both ends then it will be deleted.
		 * \param nodename the name of the node
		 * \param portname the name of the port
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if nodename does not exist.
		 */
		void RemoveReadEndpoint(const std::string &nodename,
				const std::string &portname);
		/**
		 * This function is for the node to get its QueueReader.
		 * \note This function is designed to only be called by
		 * a node.
		 * \param nodename the name of the node
		 * \param portname the name of the port
		 * \return the QueueReader for the given node and port
		 */
		QueueReader* GetReader(const std::string &nodename,
				const std::string &portname);

		/**
		 * This function is for the node to get its QueueWriter.
		 * \note This function is designed to only be called by a node.
		 * \param nodename the name of the node
		 * \param portname the name of the node
		 * \return the QueueWriter for the given node and port
		 */
		QueueWriter* GetWriter(const std::string &nodename,
				const std::string &portname);

		/**
		 * \return the attributed passed in the constructor.
		 */
		const KernelAttr &GetAttr(void) const { return kattr; }

		/**
		 * NodeBase calls this function when the user supplied
		 * process function returns.
		 * \param nodename the name of the node.
		 */
		void NodeShutdown(const std::string &nodename);
		
		/**
		 * QueueInfo calls this function when it has determined
		 * that the queue will not be used again. I.e. both endpoints
		 * have been connected and are now disconnected.
		 * \param queuename the name of the queue to delete
		 */
		void QueueShutdown(const std::string &queuename);

	private:
		Kernel(const Kernel&);
		Kernel &operator=(const Kernel&);


		void InternalWait(void);
		void ReadyOrRunningCheck(void);
		NodeInfo* GetNodeInfo(const std::string& name);
		QueueInfo* GetQueueInfo(const std::string& name);

		const KernelAttr kattr;

		mutable Sync::ReentrantLock lock;
		Sync::StatusHandler<bool> cleanupStatus;

		std::map<std::string, NodeInfo*> nodeMap;
		std::map<std::string, QueueInfo*> queueMap;
		std::deque<NodeInfo*> nodesToDelete;
		std::deque<QueueInfo*> queuesToDelete;

		/**
		 * \warning All functions in the Kernel that
		 * hold the lock must never call the wait function.
		 */
		Sync::StatusHandler<Status_t> statusHandler;

		/// Temporary unique id generator counter.
		ulong idcounter;
		/// Temporary 'unique' id generator.
		ulong GenerateId(const std::string& name);
	};
}
#endif
