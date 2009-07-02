/** \file
 * \brief The sieve producer.
 */

#ifndef PRODUCERNODE_H
#define PRODUCERNODE_H

#include "NodeBase.h"

#define SIEVE_PRODUCERNODE_TYPENAME "SieveProducerNodeTypeName"

/**
 * \brief The producer node for the simple sieve.
 *
 * This node
 * produces a ramp output starting at 2 and
 * going until cutoff is reached. If cutoff is
 * zero then go until terminated.
 * If cutoff is reached then this node will place
 * a zero onto the output before exiting.
 *
 * cutoff should be passed as an unsigned long
 * in the arg paramter.
 *
 * The output port is labeled y.
 */
class ProducerNode : public CPN::NodeBase {
public:
	ProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr, const unsigned long cutoff_)
		: CPN::NodeBase(ker, attr), cutoff(cutoff_) {}
	void Process(void);

	static void RegisterNodeType(void);
private:
	const unsigned long cutoff;
};

#endif

