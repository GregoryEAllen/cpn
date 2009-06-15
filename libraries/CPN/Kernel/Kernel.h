/** \file
 * \brief Definition for the kernel object.
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H

#include "NodeAttr.h"
#include "QueueAttr.h"
#include <string>

namespace ::CPN {
	class NodeBase;
	class QueueBase;

	class Kernel {
	public:
		Kernel();
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

	private:
	};
}
#endif
