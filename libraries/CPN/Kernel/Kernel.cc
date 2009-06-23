/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"
#include "QueueReader.h"
#include "QueueWriter.h"
#include "NodeBase.h"
#include "QueueBase.h"
#include "NodeInfo.h"
#include "QueueInfo.h"



CPN::Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr), idcounter(0) {}

CPN::Kernel::~Kernel() {}


void CPN::Kernel::Wait(void) {}

void CPN::Kernel::CreateNode(const ::std::string &nodename,
		const ::std::string &nodetype,
		const void* const arg,
		const ulong argsize) {
	PthreadMutexProtected plock(lock); 
	// Verify that nodename doesn't already exist.
	if (nodeMap[nodename]) return;
	// Create the NodeAttr object.
	CPN::NodeAttr attr(GenerateId(nodename), nodename, nodetype);
	// Create the NodeInfo structure (which creates the node)
	CPN::NodeInfo* nodeinfo = new CPN::NodeInfo(*this, attr, arg, argsize);	
	// Put the NodeInfo into our map.
	nodeMap[nodename] = nodeinfo;
}

void CPN::Kernel::CreateQueue(const ::std::string &queuename,
		const ::std::string &queuetype,
		const ulong queueLength,
		const ulong maxThreshold,
		const ulong numChannels) {
	PthreadMutexProtected plock(lock); 
	// Verify that queuename doesn't already exist.
	if (queueMap[queuename]) return;
	// Generate the QueueAttr object.
	CPN::QueueAttr attr(GenerateId(queuename), queuename, queuetype, queueLength, maxThreshold, numChannels);
	// Create the QueueInfo which creates the queue
	CPN::QueueInfo* queueinfo = new CPN::QueueInfo(attr);
	// Put QueueInfo in our map.
	queueMap[queuename] = queueinfo;
}

void CPN::Kernel::ConnectWriteEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate that qname and nodename exist
	// Lookup the queue
	// Unregister the write end of the queue from it's registered
	// place if it is already registered.
	// Register the write end with the new place.
}

void CPN::Kernel::ConnectReadEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate qname and nodename.
	// Look up the queue
	// Unregister the queue from its read port if applicable.
	// Register the read port with its new port.
}

CPN::QueueReader* CPN::Kernel::GetReader(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate nodename
	// Lookup nodeinfo
	// Check if reader exists.
	// If not create new reader and add it.
	// return the reader.
	return 0;
}

CPN::QueueWriter* CPN::Kernel::GetWriter(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate nodename
	// lookup nodeinfo.
	// check if writer exists.
	// if not create new writer and add it.
	// return the writer.
	return 0;
}

void CPN::Kernel::NodeTerminated(const NodeAttr &attr) {
	PthreadMutexProtected plock(lock); 
	// Lookup the nodeinfo
	// Unregister all ports.
	// delete the nodeinfo object.
}

ulong CPN::Kernel::GenerateId(const ::std::string& name) {
	return idcounter++;
}

