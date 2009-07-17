/** \file
 * \brief Definitions of the queue datatypes.
 */

#include "QueueDatatypes.h"

CPN::ulong CPN::QueueDatatype::numTypes = 0;
const CPN::QueueDatatype CPN::QueueDatatype::DATATYPES[] = {
	CPN::QueueDatatype("void", 1),
	CPN::QueueDatatype("int8", 1),
	CPN::QueueDatatype("sint8", 1),
	CPN::QueueDatatype("uint8", 1),
	CPN::QueueDatatype("int16", 2),
	CPN::QueueDatatype("sint16", 2),
	CPN::QueueDatatype("uint16", 2),
	CPN::QueueDatatype("int32", 4),
	CPN::QueueDatatype("sint32", 4),
	CPN::QueueDatatype("uint32", 4),
	CPN::QueueDatatype("int64", 8),
	CPN::QueueDatatype("sint64", 8),
	CPN::QueueDatatype("uint64", 8),
	CPN::QueueDatatype("float32", 4),
	CPN::QueueDatatype("float64", 8),
	CPN::QueueDatatype("complex_int8", 2),
	CPN::QueueDatatype("complex_sint8", 2),
	CPN::QueueDatatype("complex_uint8", 2),
	CPN::QueueDatatype("complex_int16", 4),
	CPN::QueueDatatype("complex_sint16", 4),
	CPN::QueueDatatype("complex_uint16", 4),
	CPN::QueueDatatype("complex_int32", 8),
	CPN::QueueDatatype("complex_sint32", 8),
	CPN::QueueDatatype("complex_uint32", 8),
	CPN::QueueDatatype("complex_int64", 16),
	CPN::QueueDatatype("complex_sint64", 16),
	CPN::QueueDatatype("complex_uint64", 16),
	CPN::QueueDatatype("complex_float32", 8),
	CPN::QueueDatatype("complex_float64", 16)
};

// String compare on a bunch of string with common prefixes... yuck
// this could be better. The question is, does it matter?
const CPN::QueueDatatype *CPN::QueueDatatype::FromName(const std::string &name) {
	for (CPN::ulong i = 0; i < numTypes; ++i) {
		if (name == DATATYPES[i].name) {
			return &DATATYPES[i];
		}
	}
	return 0;
}

