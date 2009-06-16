/** \file
 * \brief Definition for the kernel object.
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H

#include "NodeAttr.h"
#include "QueueAttr.h"
#include "KernelAttr.h"
#include <string>

namespace CPN {
	// Forward class declarations.
	class NodeBase;
	class QueueBase;

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
		Kernel(const KernelAttr &kattr);
		~Kernel();
		void Start(void);
		void Wait(void);
		void Terminate(void);
		
		NodeBase* CreateNode(const NodeAttr &nattr);
		NodeBase* GetNode(const ::std::string &name) const;
		NodeBase* GetNode(const ulong id) const;

		QueueBase* CreateQueue(const QueueAttr &qattr);
		QueueBase* GetQueue(const ::std::string &name) const;
		QueueBase* GetQueue(const ulong id) const;

		void NodeTerminated(NodeBase* node);

		const KernelAttr &GetAttr(void) const { return kattr; }
	private:
		const KernelAttr kattr;
	};
}
#endif
