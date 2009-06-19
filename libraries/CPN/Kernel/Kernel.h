/** \file
 * \brief Definition for the kernel object.
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H

#include "NodeAttr.h"
#include "QueueAttr.h"
#include "KernelAttr.h"
#include <string>
#include <map>

namespace CPN {

	class QueueReader;
	class QueueWriter;
	class QueueBase;
	class NodeInfo;
	class NodeBase;

	typedef NodeBase* (*NodeFactory_t)(const void* const);
	typedef void (*NodeDestructor_t)(NodeBase*);
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
		
		void RegisterNodeType(const ::std::string &nodetype,
				NodeFactory_t factory,
				NodeDestructor_t destructor);

		void CreateNode(const ::std::string &nodename,
				const ::std::string &nodetype,
				const void* const arg);

		void CreateQueue(const ::std::string &queuename,
				const ::std::string &queuetype);

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
		::std::map<std::string, QueueBase*> queueMap;
		::std::map<std::string, std::pair<NodeFactory_t, NodeDestructor_t> > factoryMap;
	};
}
#endif
