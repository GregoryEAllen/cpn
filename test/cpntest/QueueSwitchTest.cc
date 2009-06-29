
#include "QueueSwitchTest.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "ThresholdQueueFactory.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "Semaphore.h"
#include <vector>
#include <string>
#include <cppunit/TestAssert.h>

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif


CPPUNIT_TEST_SUITE_REGISTRATION( QueueSwitchTest );
/*
 * For ReaderSwitch
 * A -> B
 *      C
 * Then mid way
 *      B
 * A -> C
 *
 * For WriterSwitch
 *
 * A -> C
 * B
 * Then mid way
 * A
 * B -> C
 *
 */
enum NMode_t {
	PRODUCER,
	SWITCH_PRODUCER,
	CONSUMER,
	SWITCH_CONSUMER
};

struct NodeParam {
	NodeParam(const NMode_t mode_, std::vector<unsigned long>* buffer_,
			const std::string& qname_, const std::string& othername_,
			Sync::Semaphore *sema_)
	       	: mode(mode_), buffer(buffer_), qname(qname_), othername(othername_),
       		sema(sema_) {}
	const NMode_t mode;
	std::vector<unsigned long> *buffer;
	const std::string qname;
	const std::string othername;
	Sync::Semaphore *sema;
};

class SwitchNode : public CPN::NodeBase {
public:
	SwitchNode(CPN::Kernel& ker, const CPN::NodeAttr& attr, NodeParam& param_);
	void Process(void);
	static const unsigned long ENDOFQUEUE;
private:
	NodeParam param;
};

const unsigned long SwitchNode::ENDOFQUEUE = -1;

SwitchNode::SwitchNode(CPN::Kernel& ker, const CPN::NodeAttr& attr,
		NodeParam& param_) : CPN::NodeBase(ker, attr), param(param_) { }

void SwitchNode::Process(void) {
	std::string ourname = GetAttr().GetName();
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(ourname, "x");
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(ourname, "y");
	unsigned long value = 0;
	switch (param.mode) {
	case PRODUCER:
		out.Enqueue(&param.buffer->at(0), param.buffer->size());
		param.sema->Post();
		break;
	case SWITCH_PRODUCER:
		out.Enqueue(&param.buffer->at(0), param.buffer->size());
		//switch to other here
		kernel.ConnectWriteEndpoint(param.qname, param.othername, "y");
		param.sema->Post();
		out.Enqueue(&value, 1);
		break;
	case CONSUMER:
		do {
			in.Dequeue(&value, 1);
			if (value != ENDOFQUEUE)
				param.buffer->push_back(value);
		} while (value != ENDOFQUEUE);
		param.sema->Post();
		break;
	case SWITCH_CONSUMER:
		do {
			in.Dequeue(&value, 1);
			if (value != ENDOFQUEUE)
				param.buffer->push_back(value);
		} while (value != ENDOFQUEUE);
		// Switch to other here
		kernel.ConnectReadEndpoint(param.qname, param.othername, "x");
		param.sema->Post();
		in.Dequeue(&value, 1);
		param.buffer->push_back(value);
		break;
	}
}

class SwitchNodeFactory : public CPN::NodeFactory {
public:
	static const char* const NAME;

	SwitchNodeFactory() : CPN::NodeFactory(NAME) {
	}
	/// argsize should be 0 and 
	// arg should be a ptr to a NodeParam structure
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
		       	const void* const arg, const CPN::ulong argsize) {
		NodeParam* param = (NodeParam*) arg;
		return new SwitchNode(ker, attr, *param);
	}

	void Destroy(CPN::NodeBase* node) {
		delete node;
	}

	static CPN::NodeFactory* GetInstance() { return &instance; }
private:
	static SwitchNodeFactory instance;
};

const char* const SwitchNodeFactory::NAME = "SwitchNode";
SwitchNodeFactory SwitchNodeFactory::instance;



void QueueSwitchTest::setUp(void) {
	kernel = new CPN::Kernel(CPN::KernelAttr(1, "Testkernel"));
	CPNRegisterNodeFactory(SwitchNodeFactory::GetInstance());
}

void QueueSwitchTest::tearDown(void) {
	delete kernel;
	kernel = 0;
}

/*
 * For ReaderSwitch
 * A -> B
 *      C
 * Then mid way
 *      B
 * A -> C
 */ 
void QueueSwitchTest::ReaderSwitch(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	std::vector<unsigned long> listA;
	std::vector<unsigned long> listB;
	std::vector<unsigned long> listC;
	Sync::Semaphore sema;
	unsigned long firstSegmentSize = 10;
	unsigned long secondSegmentSize = 20;
	for (unsigned long i = 0; i < firstSegmentSize; ++i) {
		listA.push_back(i);
	}
	listA.push_back(SwitchNode::ENDOFQUEUE);
	for (unsigned long i = 0; i < secondSegmentSize; ++i) {
		listA.push_back(i);
	}
	listA.push_back(SwitchNode::ENDOFQUEUE);
	NodeParam paramA(PRODUCER, &listA, "", "", &sema);
	NodeParam paramB(SWITCH_CONSUMER, &listB, "thequeue", "C", &sema);
	NodeParam paramC(CONSUMER, &listC, "", "", &sema);
	kernel->CreateNode("A", SwitchNodeFactory::NAME, &paramA, 0);
	kernel->CreateNode("B", SwitchNodeFactory::NAME, &paramB, 0);
	kernel->CreateNode("C", SwitchNodeFactory::NAME, &paramC, 0);
	kernel->CreateQueue("thequeue", CPN_QUEUETYPE_THRESHOLD, 1024, 1024, 1);
	kernel->ConnectWriteEndpoint("thequeue", "A", "y");
	kernel->ConnectReadEndpoint("thequeue", "B", "x");
	kernel->Start();
	for (int i = 0; i < 3; ++i) {
		sema.Wait();
	}
	kernel->Terminate();
	kernel->Wait();
	CPPUNIT_ASSERT_EQUAL(listB.size(), firstSegmentSize);
	for (unsigned long i = 0; i < firstSegmentSize; ++i) {
		CPPUNIT_ASSERT_EQUAL(listB[i], i);
	}
	CPPUNIT_ASSERT_EQUAL(listC.size(), secondSegmentSize);
	for (unsigned long i = 0; i < secondSegmentSize; ++i) {
		CPPUNIT_ASSERT_EQUAL(listC[i], i);
	}
}

void QueueSwitchTest::WriterSwitch(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	CPPUNIT_FAIL("Unimplemented.");
}
