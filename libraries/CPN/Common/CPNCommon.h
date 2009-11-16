//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \brief Common definitions to the entire CPN library.
 * \author John Bridgman
 */

#ifndef CPN_COMMON_H
#define CPN_COMMON_H
#pragma once

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

#include <stdint.h>
#include <tr1/memory>
#include <memory>
/**
 * Namespace that seperates all the CPN specific
 * names from the global namespace.
 */
namespace CPN {

    using std::tr1::shared_ptr;
    using std::tr1::weak_ptr;
    using std::tr1::dynamic_pointer_cast;
    using std::auto_ptr;

    // Forward declarations
    class Kernel;
    class KernelImpl;
    class KernelAttr;

    class QueueBase;
    class QueueReader;
    class QueueWriter;
    class QueueAttr;
    class SimpleQueueAttr;

    class NodeAttr;
    class NodeBase;
    class NodeFactory;

    class Database;

    class SocketEndpoint;

    class StreamEndpoint;
    class KernelStream;
    class Stream;
    class UnknownStream;

    // Global enums
    enum QueueHint_t { QUEUEHINT_DEFAULT, QUEUEHINT_THRESHOLD };
    enum Error_t { E_NONE, E_EXISTS, E_NOTEXISTS };

    // Types
    typedef uint64_t Key_t;

}

#endif

