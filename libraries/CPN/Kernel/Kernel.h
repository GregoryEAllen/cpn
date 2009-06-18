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

	class QueueReader;
	class QueueWriter;

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
		
		void CreateNode(const NodeAttr &nattr);

		void CreateQueue(const QueueAttr &qattr);

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
	};
}
#endif
