/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"
#include "QueueReader.h"
#include "QueueWriter.h"
#include "NodeBase.h"
#include "QueueBase.h"
#include "NodeInfo.h"
#include "BlockingQueueWriter.h"
#include "BlockingQueueReader.h"


using namespace CPN;

Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr) {}

Kernel::~Kernel() {}


void Kernel::Wait(void) {}

void Kernel::RegisterNodeType(const ::std::string &nodetype, NodeFactory &factory) {
	// !!! Add error checking?
	nodeFactories[nodetype] = factory;
}

void Kernel::RegisterQueueType(const ::std::string &queuetype, QueueFactory &factory) {
	// !!! Add error checking?
	queueFactories[queuetype] = factory;
}

void Kernel::CreateNode(const ::std::string &nodename,
		const ::std::string &nodetype,
		const void* const arg,
		const ulong argsize) {
	// Verify that nodename doesn't already exist.
	// Verify that we know what nodetype is.
	// Create the NodeAttr object.
	// Create the NodeInfo structure (which creates the node)
	// Put the NodeInfo into our map.
}

void Kernel::CreateQueue(const ::std::string &queuename,
		const ::std::string &queuetype,
		const ulong queueLength,
		const ulong maxThreshold,
		const ulong numChannels) {
	// Verify that queuename doesn't already exist.
	// Verify that we know what queuetype is.
	// Generate the QueueAttr object.
	// Create the QueueInfo which creates the queue
	// Put QueueInfo in our map.
}

void Kernel::ConnectWriteEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	// Validate that qname and nodename exist
	// Lookup the queue
	// Unregister the write end of the queue from it's registered
	// place if it is already registered.
	// Register the write end with the new place.
}

void Kernel::ConnectReadEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	// Validate qname and nodename.
	// Look up the queue
	// Unregister the queue from it's read port if applicable.
	// Register the read port with it's new port.
}

QueueReader* Kernel::GetReader(const ::std::string &nodename,
		const ::std::string &portname) {
	// Validate nodename
	// Lookup nodeinfo
	// Check if reader exists.
	// If not create new reader and add it.
	// return the reader.
}

QueueWriter* Kernel::GetWriter(const ::std::string &nodename,
		const ::std::string &portname) {
	// Validate nodename
	// lookup nodeinfo.
	// check if writer exists.
	// if not create new writer and add it.
	// return the writer.
}

void Kernel::NodeTerminated(const NodeAttr &attr) {
	// Lookup the nodeinfo
	// Unregister all ports.
	// delete the nodeinfo object.
}


