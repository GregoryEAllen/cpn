/** \file
 * \brief Common definitions to the entire CPN library.
 */

#ifndef CPN_COMMON_H
#define CPN_COMMON_H

#include <string>

/**
 * Namespace that seperates all the CPN specific
 * names from the global namespace.
 */
namespace CPN {
	typedef unsigned long ulong;

}

/// The name for the default threshold queue type.
#define CPN_QUEUETYPE_THRESHOLD "CPN::ThresholdQueue"
#endif
