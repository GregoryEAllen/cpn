/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"

using namespace CPN;

Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr) {}

Kernel::~Kernel() {}


void Kernel::Wait(void) {}

