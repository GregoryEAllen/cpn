/** \file
 * \brief The FilterNode for the simple sieve.
 */

#ifndef FILTERNODE_H
#define FILTERNODE_H

#include "NodeBase.h"

#define SIEVE_FILTERNODE_TYPENAME "SieveFilterNode"

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
    FilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
private:
    void Process(void);
};

#endif

