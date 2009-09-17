/** \file
 * An exception to be thrown from kernel methods when
 * the kernel is shutting down.
 */

#ifndef CPN_KERNELSHUTDOWNEXCEPTION_H
#define CPN_KERNELSHUTDOWNEXCEPTION_H

#include "common.h"
#include <exception>

namespace CPN {

	/**
	 * \brief An exception indicating that the Kernel has shut down.
	 */
	class CPN_API KernelShutdownException : public std::exception {
	public:
		KernelShutdownException(const std::string& msg) throw() : message(msg) {}
		~KernelShutdownException() throw() {}
		const char* what() const throw() { return message.c_str(); }
	private:
		const std::string message;

	};
}

#endif
