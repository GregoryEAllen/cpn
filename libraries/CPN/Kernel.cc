/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"
#include "QueueReader.h"
#include "QueueWriter.h"
#include "NodeQueueReader.h"
#include "NodeQueueWriter.h"
#include "NodeBase.h"
#include "QueueBase.h"
#include "NodeInfo.h"
#include "QueueInfo.h"
#include "KernelShutdownException.h"
#include "MapDeleter.h"
#include "MapInvoke.h"
#include <algorithm>
#include <cassert>
#include <vector>
#include <stdexcept>

CPN::Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr), statusHandler(READY), idcounter(0) {}

CPN::Kernel::~Kernel() {
	Terminate();
	// Delete any remaining objects.
	std::vector<std::pair<std::string, CPN::NodeInfo*> > nodelist;
	std::vector<std::pair<std::string, CPN::QueueInfo*> > queuelist;
	{
		PthreadMutexProtected plock(lock); 
		InternalWait();
		queuelist.assign(queueMap.begin(), queueMap.end());
		queueMap.clear();
	}
	for_each(nodelist.begin(), nodelist.end(), MapDeleter<std::string, CPN::NodeInfo>());
	for_each(queuelist.begin(), queuelist.end(), MapDeleter<std::string, CPN::QueueInfo>());
}

void CPN::Kernel::Start(void) {
	statusHandler.CompareAndPost(READY, RUNNING);
}

void CPN::Kernel::Wait(void) {
	PthreadMutexProtected plock(lock); 
	InternalWait();
}

/**
 * Wait untill all nodes have terminated.
 * Also deletes all the terminated nodes.
 *
 * Post condition: nodeMap is empty.
 */
void CPN::Kernel::InternalWait(void) {
	while (true) {
		while (nodesToDelete.size()) {
			NodeInfo* nodeinfo = nodesToDelete.front();
			nodesToDelete.pop_front();
			delete nodeinfo;
		}
		if (0 == nodeMap.size()) break;
		nodeTermination.Wait(lock);
	}
	statusHandler.Post(STOPPED);
}

void CPN::Kernel::Terminate(void) {
	PthreadMutexProtected plock(lock); 
	if (STOPPED == statusHandler.Get()) return;
	for_each(nodeMap.begin(), nodeMap.end(),
		       	MapInvoke<std::string, CPN::NodeInfo, void(CPN::NodeInfo::*)(void)>(
				&CPN::NodeInfo::Terminate));
	statusHandler.Post(TERMINATING);
}

void CPN::Kernel::CreateNode(const ::std::string &nodename,
		const ::std::string &nodetype,
		const void* const arg,
		const ulong argsize) {
	PthreadMutexProtected plock(lock); 
	ReadyOrRunningCheck();
	// Verify that nodename doesn't already exist.
	if (nodeMap.find(nodename) != nodeMap.end()) return;
	CPN::NodeAttr attr(GenerateId(nodename), nodename, nodetype);
	nodeMap.insert(make_pair(nodename, new CPN::NodeInfo(*this, attr, arg, argsize)));
}

void CPN::Kernel::CreateQueue(const ::std::string &queuename,
		const ::std::string &queuetype,
		const ulong queueLength,
		const ulong maxThreshold,
		const ulong numChannels) {
	PthreadMutexProtected plock(lock); 
	ReadyOrRunningCheck();
	// Verify that queuename doesn't already exist.
	if (queueMap.find(queuename) != queueMap.end()) return;
	CPN::QueueAttr attr(GenerateId(queuename), queuename, queuetype,
		       	queueLength, maxThreshold, numChannels);
	queueMap.insert(make_pair(queuename, new CPN::QueueInfo(attr)));
}

void CPN::Kernel::ConnectWriteEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	ReadyOrRunningCheck();
	// Validate that qname and nodename exist
	QueueInfo* qinfo = GetQueueInfo(qname);
	NodeInfo* ninfo = GetNodeInfo(nodename);
	// Unregister the write end of the queue from it's registered
	// place if it is already registered.
	// Register the write end with the new place.
	ninfo->SetWriter(qinfo, portname);
}

void CPN::Kernel::ConnectReadEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	ReadyOrRunningCheck();
	// Validate qname and nodename.
	QueueInfo* qinfo = GetQueueInfo(qname);
	NodeInfo* ninfo = GetNodeInfo(nodename);
	// Unregister the queue from its read port if applicable.
	// Register the read port with its new port.
	ninfo->SetReader(qinfo, portname);
}

CPN::QueueReader* CPN::Kernel::GetReader(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	ReadyOrRunningCheck();
	// Validate nodename
	// Lookup nodeinfo
	NodeInfo* ninfo = GetNodeInfo(nodename);
	// Check if reader exists.
	// If not create new reader and add it.
	// return the reader.
	QueueReader* qreader = ninfo->GetReader(portname);
	assert(qreader);
	return qreader;
}

CPN::QueueWriter* CPN::Kernel::GetWriter(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	ReadyOrRunningCheck();
	// Validate nodename
	// lookup nodeinfo.
	NodeInfo* ninfo = GetNodeInfo(nodename);
	// check if writer exists.
	// if not create new writer and add it.
	// return the writer.
	QueueWriter* qwriter = ninfo->GetWriter(portname);
	assert(qwriter);
	return qwriter;
}

void CPN::Kernel::NodeShutdown(const std::string &nodename) {
	PthreadMutexProtected plock(lock); 
	NodeInfo* ninfo = nodeMap[nodename];
	nodeMap.erase(nodename);
	nodesToDelete.push_back(ninfo);
	nodeTermination.Signal();
}

void CPN::Kernel::ReadyOrRunningCheck(void) {
	Status_t currentStatus = statusHandler.Get();
	if (currentStatus != READY && currentStatus != RUNNING) {
		throw CPN::KernelShutdownException("Kernel shutting down.");
	}
}

CPN::ulong CPN::Kernel::GenerateId(const ::std::string& name) {
	return idcounter++;
}

CPN::NodeInfo* CPN::Kernel::GetNodeInfo(const std::string& name) {
	std::map<std::string, NodeInfo*>::iterator nodeItr;
	nodeItr = nodeMap.find(name);
	if (nodeItr == nodeMap.end() || (*nodeItr).second == 0) {
		throw std::invalid_argument("No such node: " + name);
	}
	return (*nodeItr).second;
}

CPN::QueueInfo* CPN::Kernel::GetQueueInfo(const std::string& name) {
	std::map<std::string, QueueInfo*>::iterator queueItr;
	queueItr = queueMap.find(name);
	if (queueItr == queueMap.end() || (*queueItr).second == 0) {
		throw std::invalid_argument("No such queue: " + name);
	}
	return (*queueItr).second;
}

