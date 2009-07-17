/** \file
 * \brief Defintions of and helper functions for queue datatypes.
 */

#ifndef CPN_QUEUEDATATYPES_H
#define CPN_QUEUEDATATYPES_H

#include "common.h"

namespace CPN {
	class CPN_API QueueDatatype {
	public:
		~QueueDatatype() {}
		const char* const Name(void) const { return name; }
		const ulong Size(void) const { return size; }
		static const QueueDatatype *FromName(const std::string &name);
	private:
		struct data {
			const char* const name;
			const ulong size;
		};

		QueueDatatype(const char* const name_, const ulong size_)
			: name(name_), size(size_) { ++numTypes; }
		QueueDatatype(const QueueDatatype&);
		const QueueDatatype &operator=(const QueueDatatype&) const;
		const char* const name;
		const ulong size;
		static const QueueDatatype DATATYPES[];
		static ulong numTypes;
	};
}

#endif

