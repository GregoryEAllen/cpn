/** \file
 * \brief Definition for the kernel object.
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H

#include "KernelAttr.h"
#include "NodeFactory.h"
#include "QueueFactory.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
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
		 * Wait for all nodes to terminate.
		 * Has no effect if not started.
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
		 */
		void CreateNode(const ::std::string &nodename,
				const ::std::string &nodetype,
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
		 */
		void CreateQueue(const ::std::string &queuename,
				const ::std::string &queuetype,
				const ulong queueLength,
				const ulong maxThreshold,
				const ulong numChannels);

		/**
		 * Connects the write endpoint of the given queue to the
		 * given port on the given node.
		 * \param qname the name of the queue to connect
		 * \param nodename the name of the node to connect
		 * \param portname the name of the writer port on the node
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if the queue or node
		 * do not exist.
		 */
		void ConnectWriteEndpoint(const ::std::string &qname,
				const ::std::string &nodename,
				const ::std::string &portname);

		/**
		 * Connects the reader endpoint of the given queue to the
		 * given port on the given node.
		 * \param qname the name of the queue to connect
		 * \param nodename the name of the node to connect
		 * \param portname the name of the reader port on the node
		 * \throws KernelShutdownException if Wait completed or
		 * Terminate has been called.
		 * \throws std::invalid_argument if the queue or node
		 * do not exist.
		 */
		void ConnectReadEndpoint(const ::std::string &qname,
				const ::std::string &nodename,
				const ::std::string &portname);

		/**
		 * This function is for the node to get its QueueReader.
		 * \note This function is designed to only be called by
		 * a node.
		 * \param nodename the name of the node
		 * \param portname the name of the port
		 * \return the QueueReader for the given node and port
		 */
		QueueReader* GetReader(const ::std::string &nodename,
				const ::std::string &portname);

		/**
		 * This function is for the node to get its QueueWriter.
		 * \note This function is designed to only be called by a node.
		 * \param nodename the name of the node
		 * \param portname the name of the node
		 * \return the QueueWriter for the given node and port
		 */
		QueueWriter* GetWriter(const ::std::string &nodename,
				const ::std::string &portname);

		/**
		 * \return the attributed passed in the constructor.
		 */
		const KernelAttr &GetAttr(void) const { return kattr; }

		/**
		 * NodeBase calls this function when the user supplied
		 * process function returns.
		 * \param nodename the name of the node.
		 */
		void NodeShutdown(const ::std::string &nodename);
	private:

		ulong GenerateId(const ::std::string& name);

		PthreadMutex lock;
		PthreadCondition nodeTermination;

		const KernelAttr kattr;

		::std::map<std::string, NodeInfo*> nodeMap;
		::std::map<std::string, QueueInfo*> queueMap;

		Kernel(const Kernel&);
		Kernel &operator=(const Kernel&);

		ulong idcounter;
		enum Status_t { INITIALIZED, STARTED, STOPPED, SHUTTINGDOWN };
		Status_t status;

		std::deque<NodeInfo*> nodesToDelete;
	};
}
#endif
