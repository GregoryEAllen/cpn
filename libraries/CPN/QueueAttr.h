/** \file
 * \brief Definition of the queue attributes.
 */

#ifndef CPN_QUEUEATTR_H
#define CPN_QUEUEATTR_H

#include "common.h"

namespace CPN {
	
	class QueueDatatype;
	/**
	 * \brief The attributes for a queue.
	 *
	 */
	class CPN_API QueueAttr {
	public:
		QueueAttr(const ulong id_,
			       	const ::std::string &name_,
				const ::std::string &queuetype_,
				const ulong queueLength_,
				const ulong maxThreshold_,
				const ulong numChannels_ = 1)
			: id(id_), name(name_), queueLength(queueLength_),
			maxThreshold(maxThreshold_), numChannels(numChannels_),
			queuetype(queuetype_)	{}

		ulong GetID(void) const { return id; }

		const std::string &GetName(void) const { return name; }

		const ulong GetLength(void) const { return queueLength; }
		void SetLength(ulong l) { queueLength = l; }

		const ulong GetMaxThreshold(void) const { return maxThreshold; }
		void SetMaxThreshold(ulong l) { maxThreshold = l; }

		const ulong GetNumChannels(void) const { return numChannels; }

		const ::std::string &GetTypeName(void) const { return queuetype; }

		const QueueDatatype* GetDatatype(void) const { return datatype; }
	private:
		const ulong id;
		const std::string name;
		ulong queueLength;
		ulong maxThreshold;
		const ulong numChannels;
		const std::string queuetype;
		const QueueDatatype* datatype;
	};
}
#endif
