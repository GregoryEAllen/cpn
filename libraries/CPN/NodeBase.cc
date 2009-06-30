/** \file
 * Implementation for the NodeBase class.
 */

#include "NodeBase.h"
#include "Kernel.h"
#include "KernelShutdownException.h"

void* CPN::NodeBase::EntryPoint(void) {
	Kernel::Status_t status = kernel.CompareStatusAndWait(Kernel::READY);
	if (status == Kernel::RUNNING) {
		try {
			Process();
		} catch (CPN::KernelShutdownException e) {
		}
	}
	kernel.NodeShutdown(GetAttr().GetName());
	return 0;
}

