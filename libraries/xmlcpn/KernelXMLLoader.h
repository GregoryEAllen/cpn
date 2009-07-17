/** \file
 */

#ifndef CPN_KERNELXMLLOADER_H
#define CPN_KERNELXMLLOADER_H

#include "Kernel.h"
#include <string>

namespace xmlpp {
	class Node;
}

namespace CPN {
	class KernelXMLLoader {
	public:
		KernelXMLLoader(const std::string &filename);
		~KernelXMLLoader();
		KernelAttr GetKernelAttr(void);
		void SetupNodes(Kernel &kernel);
		void SetupQueues(Kernel &kernel);
	private:
		void AddNode(Kernel &kernel, xmlpp::Node* node);
		void AddQueue(Kernel &kernel, xmlpp::Node* node);
		class pimpl_t;
		pimpl_t *pimpl;
	};
}

#endif

