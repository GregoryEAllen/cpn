/** \file
 * \brief The FilterNode for the simple sieve.
 */

#ifndef FILTERNODE_H
#define FILTERNODE_H

#include "NodeBase.h"

#define SIEVE_FILTERNODE_TYPENAME "SieveFilterNodeTypeName"

/**
 * \brief This node implements the filter for the
 * simple sieve.
 *
 * The prime to filter out of the stream is
 * passed as an unsigned long to the arg paramter.
 *
 * The input for this node is x.
 * The output for this node is y.
 *
 * This node assumes that if it reads a 0 then
 * that means all work is done.
 */
class FilterNode : public CPN::NodeBase {
public:
	struct Param {
		unsigned long filterval;
		unsigned long threshold;
	};
	FilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr, unsigned long filterval_,
			unsigned long threshold_)
		: CPN::NodeBase(ker, attr), filterval(filterval_),
       		threshold(threshold_) {}
	void Process(void);
	static void RegisterNodeType(void);
private:
	const unsigned long filterval;
	const unsigned long threshold;
};

#endif

