/** \file
 * \brief The sieve producer.
 */

#ifndef PRODUCERNODE_H
#define PRODUCERNODE_H

#include "NodeBase.h"
#include "SieveControllerNode.h"

#define SIEVE_PRODUCERNODE_TYPENAME "SieveProducerNode"

/**
 * \brief The producer node for the simple sieve.
 *
 * This node
 * produces a ramp output starting at 2 and
 * going until numberBound is reached. If numberBound is
 * zero then go until terminated.
 * If numberBound is reached then this node will place
 * a zero onto the output before exiting.
 *
 * If threshold is greater than 1 then this will produces
 * threshold - 1 elements at a time with a number at the
 * front of how many elements it produced. (The last
 * production could be smaller than maximum.)
 *
 * The output port is labeled y.
 */
class ProducerNode : public CPN::NodeBase {
public:
	ProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
private:
	void Process(void);
    unsigned long numberBound;
};

#endif

