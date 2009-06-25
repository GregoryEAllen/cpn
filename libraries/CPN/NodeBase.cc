/** \file
 * Implementation for the NodeBase class.
 */

#include "NodeBase.h"
#include "Kernel.h"

void* CPN::NodeBase::EntryPoint(void) {
	Process();
	return 0;
}

