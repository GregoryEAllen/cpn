
#include "SieveTest.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "NodeAttr.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "ToString.h"
#include "AutoBuffer.h"
#include <cppunit/TestAssert.h>


CPPUNIT_TEST_SUITE_REGISTRATION( SieveTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

/*
 * A test after the Sieve of Eratosthenes.
 * A lot of things have to be working for this to pass.
 */

const unsigned long PRIMES[] = {
2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
};

const unsigned long NUMPRIMES = sizeof(PRIMES)/sizeof(unsigned long);

const unsigned long MAX_PRIME_VALUE = 100;

class SieveResultNode : public CPN::NodeBase {
public:
	SieveResultNode(CPN::Kernel& ker, const CPN::NodeAttr& attr, unsigned long* result_,
			unsigned long resultsize_);
	void Process(void);
private:
	unsigned long* result;
	unsigned long resultsize;
};
SieveResultNode::SieveResultNode(CPN::Kernel& ker, const CPN::NodeAttr& attr,
	       	unsigned long* result_,
		unsigned long resultsize_)
	: CPN::NodeBase(ker, attr), result(result_), resultsize(resultsize_)
{
}

void SieveResultNode::Process(void) {
	std::string ourname = GetAttr().GetName();
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(ourname, "x");
	unsigned long index = 0;
	/*
	unsigned long value = 0;
	do {
		in.Dequeue(&value, 1);
		printf(" %lu,", value);
	} while (value != 0);
	*/
	while (index < resultsize) {
		in.Dequeue(&result[index], 1);
		++index;
	}
}

class SieveFilterNode : public CPN::NodeBase {
public:
	SieveFilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
	void Process(void);
private:
};

SieveFilterNode::SieveFilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
       	: CPN::NodeBase(ker, attr) {
}
void SieveFilterNode::Process(void) {
	std::string ourname = GetAttr().GetName();
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(ourname, "y");
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(ourname, "x");
	unsigned long input = 0;
	unsigned long value = 0;
	in.Dequeue(&input, 1);
	if (0 == input) {
		out.Enqueue(&input, 1);
		return;
	}
	value = input;
	out.Enqueue(&input, 1);
	// Add new node
	std::string nodename = toString("Filter %lu+", value); 
	kernel.CreateNode(nodename, "SieveFilterNode", 0, 0);
	std::string qname = toString("Queue %lu+", value);
	kernel.CreateQueue(qname, CPN_QUEUETYPE_THRESHOLD, 100, 100, 1);
	kernel.ConnectWriteEndpoint(qname, GetAttr().GetName(), "y");
	kernel.ConnectReadEndpoint(qname, nodename, "x");
	kernel.ConnectWriteEndpoint("consumerqueue", nodename, "y");
	while (true) {
		in.Dequeue(&input, 1);
		if (input%value != 0) {
			out.Enqueue(&input, 1);
		}
		if (input == 0) {
			out.Enqueue(&input, 1);
			break;
		}
	}

}

class SieveProducerNode : public CPN::NodeBase {
public:
	SieveProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
	void Process(void);
};

SieveProducerNode::SieveProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
	: CPN::NodeBase(ker, attr) {
}
void SieveProducerNode::Process(void) {
	std::string ourname = GetAttr().GetName();
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(ourname, "y");
	unsigned long index = 2;
	while (index <= MAX_PRIME_VALUE) {
		out.Enqueue(&index, 1);
		++index;
	}
	index = 0;
	out.Enqueue(&index, 1);
}

class SieveNodeFactory : public CPN::NodeFactory {
public:
	SieveNodeFactory(std::string name_) : CPN::NodeFactory(name_) {
	}
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
		       	const void* const arg, const CPN::ulong argsize) {
		if ("SieveFilterNode" == GetName()) {
			return new SieveFilterNode(ker, attr);
		}
		if ("SieveProducerNode" == GetName()) {
			return new SieveProducerNode(ker, attr);
		}
		if ("SieveResultNode" == GetName()) {
			unsigned long resultsize = argsize;
			unsigned long* result = (unsigned long*)arg;
			return new SieveResultNode(ker, attr, result, resultsize);
		}
		return 0;
	}

	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};

SieveNodeFactory SieveFilterNodeFactory("SieveFilterNode");
SieveNodeFactory SieveProducerNodeFactory("SieveProducerNode");
SieveNodeFactory SieveResultNodeFactory("SieveResultNode");

void SieveTest::setUp(void) {
	CPNRegisterNodeFactory(&SieveFilterNodeFactory);
	CPNRegisterNodeFactory(&SieveProducerNodeFactory);
	CPNRegisterNodeFactory(&SieveResultNodeFactory);
	kernel = new CPN::Kernel(CPN::KernelAttr(1, "Testing"));
}

void SieveTest::tearDown(void) {
	delete kernel;
	kernel = 0;
}

void SieveTest::RunTest(void) {
	AutoBuffer buffer(NUMPRIMES*sizeof(unsigned long));
	kernel->CreateNode("TheProducer", "SieveProducerNode", 0, 0);
	kernel->CreateNode("Filter 1+", "SieveFilterNode", 0, 0);
	kernel->CreateNode("TheResult", "SieveResultNode", buffer, NUMPRIMES);
	kernel->CreateQueue("consumerqueue", CPN_QUEUETYPE_THRESHOLD, 100, 100, 1);
	kernel->CreateQueue("producerqueue", CPN_QUEUETYPE_THRESHOLD, 100, 100, 1);
	kernel->ConnectWriteEndpoint("producerqueue", "TheProducer", "y");
	kernel->ConnectWriteEndpoint("consumerqueue", "Filter 1+", "y");
	kernel->ConnectReadEndpoint("producerqueue", "Filter 1+", "x");
	kernel->ConnectReadEndpoint("consumerqueue", "TheResult", "x");
	kernel->Start();
	unsigned long* result = (unsigned long*) buffer.GetBuffer();
	for (unsigned long i = 0; i < NUMPRIMES; i++) {
		CPPUNIT_ASSERT_EQUAL(result[i], PRIMES[i]);
	}
}


