/** \file
 * \brief Definition for the kernel object.
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H

#include "KernelAttr.h"
#include "NodeFactory.h"
#include "QueueFactory.h"
#include <string>
#include <map>

namespace CPN {

	class QueueReader;
	class QueueWriter;
	class NodeInfo;
	class QueueInfo;

	/**
	 * \brief The Kernel declaration.
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
		 * Wait for the process network to end.
		 * May wait 'forever'.
		 */
		void Wait(void);
		
		void RegisterNodeType(const ::std::string &nodetype, NodeFactory &factory);

		void RegisterQueueType(const ::std::string &queuetype, QueueFactory &factory);

		void CreateNode(const ::std::string &nodename,
				const ::std::string &nodetype,
				const void* const arg,
				const ulong argsize);

		void CreateQueue(const ::std::string &queuename,
				const ::std::string &queuetype,
				const ulong queueLength,
				const ulong maxThreshold,
				const ulong numChannels);

		void ConnectWriteEndpoint(const ::std::string &qname,
				const ::std::string &nodename,
				const ::std::string &portname);

		void ConnectReadEndpoint(const ::std::string &qname,
				const ::std::string &nodename,
				const ::std::string &portname);

		QueueReader* GetReader(const ::std::string &nodename,
				const ::std::string &portname);

		QueueWriter* GetWriter(const ::std::string &nodename,
				const ::std::string &portname);

		void NodeTerminated(const NodeAttr &attr);

		const KernelAttr &GetAttr(void) const { return kattr; }
	private:
		const KernelAttr kattr;

		::std::map<std::string, NodeInfo*> nodeMap;
		::std::map<std::string, QueueInfo*> queueMap;
		::std::map<std::string, NodeFactory> nodeFactories;
		::std::map<std::string, QueueFactory> queueFactories;

		Kernel(const Kernel&) {};
		Kernel &operator=(const Kernel&) {};
	};
}
#endif
