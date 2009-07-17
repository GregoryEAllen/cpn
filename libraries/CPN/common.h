/** \file
 * \brief Common definitions to the entire CPN library.
 */

#ifndef CPN_COMMON_H
#define CPN_COMMON_H

#include <string>

// This code was obtained from http://gcc.gnu.org/wiki/Visibility
// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define CPN_HELPER_DLL_IMPORT __declspec(dllimport)
#define CPN_HELPER_DLL_EXPORT __declspec(dllexport)
#define CPN_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define CPN_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
#define CPN_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
#define CPN_HELPER_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define CPN_HELPER_DLL_IMPORT
#define CPN_HELPER_DLL_EXPORT
#define CPN_HELPER_DLL_LOCAL
#endif
#endif
// Now we use the generic helper definitions above to define CPN_API and CPN_LOCAL.
// CPN_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// CPN_LOCAL is used for non-api symbols.
#ifdef CPN_DLL // defined if CPN is compiled as a DLL
#ifdef CPN_DLL_EXPORTS // defined if we are building the CPN DLL (instead of using it)
#define CPN_API CPN_HELPER_DLL_EXPORT
#else
#define CPN_API CPN_HELPER_DLL_IMPORT
#endif // CPN_DLL_EXPORTS
#define CPN_LOCAL CPN_HELPER_DLL_LOCAL
#else // CPN_DLL is not defined: this means CPN is a static lib.
#define CPN_API
#define CPN_LOCAL
#endif // CPN_DLL

/**
 * Namespace that seperates all the CPN specific
 * names from the global namespace.
 */
namespace CPN {
	typedef unsigned long ulong;

}

/// The name for the default threshold queue type.
#define CPN_QUEUETYPE_THRESHOLD "CPN::ThresholdQueue"
#define CPN_QUEUETYPE_SIMPLE "CPN::SimpleQueue"
#endif
