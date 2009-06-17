/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"

using namespace CPN;

Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr) {}

Kernel::~Kernel() {}

void Kernel::Start(void) {}

void Kernel::Wait(void) {}

NodeBase* Kernel::CreateNode(const NodeAttr &nattr) {}

NodeBase* Kernel::GetNode(const ::std::string &name) const {}

NodeBase* Kernel::GetNode(const ulong id) const {}

QueueBase* Kernel::CreateQueue(const QueueAttr &qattr) {}

QueueBase* Kernel::GetQueue(const ::std::string &name) const {}

QueueBase* Kernel::GetQueue(const ulong id) const {}

void Kernel::NodeTerminated(NodeBase* node) {}

