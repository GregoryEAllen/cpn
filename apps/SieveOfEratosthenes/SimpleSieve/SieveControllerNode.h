/** \file
 * \brief The SieveConrollerNode
 */

#ifndef SIEVECONTROLLERNODE_H
#define SIEVECONTROLLERNODE_H

#include "NodeBase.h"
#include "QueueAttr.h"
#include <vector>

#define SIEVECONTROLLERNODE_TYPENAME "SieveControllerNode"

/**
 * \brief The SieveControllerNode controls everything
 * that happens with the simple sieve process network.
 *
 * To start the sieve process network one only needs
 * to create this one node and then start the process
 * network. This node will create the producer and 
 * all the filters.
 *
 */
class SieveControllerNode : public CPN::NodeBase {
public:
    SieveControllerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
    void Process(void);
private:
    void Initialize(void);
    void SetupQueue(const std::string& nodename);
    void CreateFilter(const unsigned long prime);
    CPN::QueueAttr GetQueueAttr();
    unsigned long primeBound;
    unsigned long numberBound;
    unsigned long queueSize;
    unsigned long lastprime;
};

#endif

