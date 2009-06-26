/** \file
 * Implementation for the NodeBase class.
 */

#include "NodeBase.h"
#include "Kernel.h"
#include "KernelShutdownException.h"

void* CPN::NodeBase::EntryPoint(void) {
	try {
		Process();
	} catch (CPN::KernelShutdownException e) {
	}
	kernel.NodeShutdown(GetAttr().GetName());
	return 0;
}

