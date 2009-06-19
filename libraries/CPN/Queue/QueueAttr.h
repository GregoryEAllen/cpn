/** \file
 * \brief Definition of the queue attributes.
 */

#ifndef CPN_QUEUEATTR_H
#define CPN_QUEUEATTR_H

#include "Attribute.h"

namespace CPN {
	
	class QueueAttr : public Attribute {
	public:
		QueueAttr(const ulong ID,
			       	const ::std::string &name,
				const ulong queueLength_,
				const ulong maxThreshold_,
				const ulong numChannels_ = 1)
			: Attribute(ID, name), queueLength(queueLength_),
	       maxThreshold(maxThreshold_), numChannels(numChannels_)	{}

		const ulong &GetLength(void) const { return queueLength; }

		const ulong &GetMaxThreshold(void) const { return maxThreshold; }

		const ulong &GetNumChannels(void) const { return numChannels; }
	private:
		const ulong queueLength;
		const ulong maxThreshold;
		const ulong numChannels;
	};
}
#endif
