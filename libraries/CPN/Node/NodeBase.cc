/** \file
 * Implementation for the NodeBase class.
 */

#include "NodeBase.h"
#include "QueueReader.h"
#include "QueueWriter.h"


CPN::NodeBase::NodeBase(Kernel &ker, const NodeAttr &attr,
			       	const ::std::string* const inputnames,
				const ulong numinputs,
			       	const ::std::string* const outputnames,
				const ulong numoutputs) :
	kernel(ker), attr(attr), inputnames(inputnames), numinputs(numinputs),
       	outputnames(outputnames), numoutputs(numoutputs)
{
}

void CPN::NodeBase::ConnectWriter(const ::std::string &portname, QueueWriter *qwriter) {
	writerqlist[portname] = qwriter;
}

CPN::QueueWriter* CPN::NodeBase::GetWriter(const ::std::string &portname) {
	return writerqlist[portname];
}

void CPN::NodeBase::ConnectReader(const ::std::string &portname, QueueReader *qreader) {
	readerqlist[portname] = qreader;
}

CPN::QueueReader* CPN::NodeBase::GetReader(const ::std::string &portname) {
	return readerqlist[portname];
}

void* CPN::NodeBase::EntryPoint(void) {
	Process();
	return 0;
}
