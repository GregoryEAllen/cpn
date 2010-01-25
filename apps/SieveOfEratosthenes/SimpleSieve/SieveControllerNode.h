/** \file
 * \brief The SieveConrollerNode
 */

#ifndef SIEVECONTROLLERNODE_H
#define SIEVECONTROLLERNODE_H

#include "NodeBase.h"
#include "QueueAttr.h"
#include <vector>

#define SIEVECONTROLLERNODE_TYPENAME "SieveControllerNodeTypeName"

/**
 * \brief The SieveControllerNode controls everything
 * that happens with the simple sieve process network.
 *
 * To start the sieve process network one only needs
 * to create this one node and then start the process
 * network. This node will create the producer and 
 * all the filters.
 * This node calls Kernel::Terminate on completion.
 *
 * This node expects a pointer to a SievecontrollerNode::Param
 * to be passed as the arg parameter in Kernel::CreateNode
 */
class SieveControllerNode : public CPN::NodeBase {
public:
	struct Param {
		/// vector to store the results of the computation in.
		std::vector<unsigned long> *results;
		/// value to go until primes found above this value
		/// may be 0 for no maximum
		unsigned long primeBound;
		/// paramter for the maximum number to consider for primes
		/// may be 0 for no maximum
		unsigned long numberBound;
		/// The type of queue to create between nodes
        CPN::QueueHint_t queuehint;
		/// The size of the queues
		unsigned long queueSize;
		/// The threshold value to use
		unsigned long threshold;
	};
	SieveControllerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			Param param_)
		: CPN::NodeBase(ker, attr), param(param_), lastprime(0) {}
	void Process(void);
private:
	void Initialize(void);
	void SetupQueue(const std::string& nodename);
	void CreateFilter(const unsigned long prime);
    CPN::QueueAttr GetQueueAttr();
	Param param;
	unsigned long lastprime;
};

#endif

