
#include "SieveTest.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "NodeAttr.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "ToString.h"
#include "AutoBuffer.h"
#include <cppunit/TestAssert.h>
#include <cmath>


CPPUNIT_TEST_SUITE_REGISTRATION( SieveTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

#if 0
#define DBPRINT(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define DBPRINT(fmt, ...)
#endif
/*
 * A test after the Sieve of Eratosthenes.
 * A lot of things have to be working for this to pass.
 */

typedef unsigned long SieveNumber;

const SieveNumber PRIMES[] = {
2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
};

const SieveNumber NUMPRIMES = sizeof(PRIMES)/sizeof(SieveNumber);

const SieveNumber MAX_PRIME_VALUE = 100;

const char PORT_IN[] = "x";
const char PORT_OUT[] = "y";
const char PORT_FORMAT[] = "Result: %lu";
const char FILTER_FORMAT[] = "Filter: %lu";
const char QUEUE_FORMAT[] = "Queue %lu";

const char FILTER_TYPE[] = "SieveFilterType";
const char RESULT_TYPE[] = "SieveResultType";
const char PRODUCER_TYPE[] = "SieveProducerType";

class SieveResultNode : public CPN::NodeBase {
public:
    struct Param {
        unsigned long* result;
        unsigned long resultsize;
    };
    SieveResultNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
    void Process();
private:
    unsigned long* result;
    unsigned long resultsize;
};
SieveResultNode::SieveResultNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr)
{
    Param *param = (Param*)attr.GetArg().GetBuffer();
    result = param->result;
    resultsize = param->resultsize/sizeof(unsigned long);
}

void SieveResultNode::Process() {
    std::string ourname = GetName();
    DBPRINT("Result node %s started\n", ourname.c_str());
    SieveNumber portnum = 0;
    CPN::QueueReaderAdapter<SieveNumber> in = GetReader(ToString(PORT_FORMAT, portnum));
    SieveNumber index = 0;
    while (index < resultsize) {
        SieveNumber value;
        in.Dequeue(&value, 1);
        if (value == 0) {
            ++portnum;
            DBPRINT("Result swapped port to %lu\n", portnum);
            in = GetReader(ToString(PORT_FORMAT, portnum));
        } else {
            result[index] = value;
            DBPRINT("Result[%lu] = %lu\n", index, value);
            ++index;
        }
    }
    SieveNumber val = -1;
    in.Dequeue(&val, 1);
    assert(0 == val);
    DBPRINT("%s stopped\n", ourname.c_str());
}

class SieveFilterNode : public CPN::NodeBase {
public:
    SieveFilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
    void Process();
    void CreateNextFilter();
private:
    SieveNumber nodenum;
};

SieveFilterNode::SieveFilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr) {
    nodenum = *((SieveNumber*)attr.GetArg().GetBuffer());
}

void SieveFilterNode::CreateNextFilter() {
    SieveNumber filternum = nodenum + 1;
    std::string nodename = ToString(FILTER_FORMAT, filternum); 
    CPN::NodeAttr nattr(nodename, FILTER_TYPE);
    nattr.SetParam(StaticBuffer(&filternum, sizeof(SieveNumber)));
    kernel.CreateNode(nattr);
    // A queue between us and next
    CPN::QueueAttr qattr(100, 100);
    qattr.SetReader(nodename, PORT_IN);
    qattr.SetWriter(GetName(), PORT_OUT);
    kernel.CreateQueue(qattr);
    // A queue between next and result
    std::string portname = ToString(PORT_FORMAT, filternum);
    qattr.SetWriter(nodename, portname);
    qattr.SetReader("TheResult", portname);
    kernel.CreateQueue(qattr);
}

void SieveFilterNode::Process() {
    std::string ourname = GetName();
    DBPRINT("Filter node %s started\n", ourname.c_str());
    CPN::QueueReaderAdapter<SieveNumber> in = GetReader(PORT_IN);
    std::string portname = ToString(PORT_FORMAT, nodenum);
    CPN::QueueWriterAdapter<SieveNumber> result = GetWriter(portname);
    CPN::QueueWriterAdapter<SieveNumber> out;
    SieveNumber input = 0;
    SieveNumber value = 0;
    in.Dequeue(&input, 1);
    result.Enqueue(&input, 1);
    if (0 == input) {
        DBPRINT("%s stopped early\n", ourname.c_str());
        return;
    }
    value = input;
    if (value < (SieveNumber)ceil(sqrt(MAX_PRIME_VALUE))) {
        input = 0;
        result.Enqueue(&input, 1);
        result.Release();

        // Add new node
        CreateNextFilter();
        out = GetWriter(PORT_OUT);
    } else {
        out = result;
    }
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
    DBPRINT("%s stopped\n", ourname.c_str());
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
    std::string ourname = GetName();
    DBPRINT("Producer node %s started\n", ourname.c_str());
    CPN::QueueWriterAdapter<unsigned long> out = GetWriter(PORT_OUT);
    unsigned long index = 2;
    while (index <= MAX_PRIME_VALUE) {
        DBPRINT("Enqueueing %lu\n", index);
        out.Enqueue(&index, 1);
        ++index;
    }
    index = 0;
    out.Enqueue(&index, 1);
    DBPRINT("%s stopped\n", ourname.c_str());
}

class SieveNodeFactory : public CPN::NodeFactory {
public:
    enum Type {
        FILTER, RESULT, PRODUCER
    };
    SieveNodeFactory(std::string name_, Type t) : CPN::NodeFactory(name_), type(t) {
    }
    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
        switch (type) {
        case FILTER:
            return CPN::shared_ptr<CPN::NodeBase>(new SieveFilterNode(ker, attr));
        case PRODUCER:
            return CPN::shared_ptr<CPN::NodeBase>(new SieveProducerNode(ker, attr));
        case RESULT:
            return CPN::shared_ptr<CPN::NodeBase>(new SieveResultNode(ker, attr));
        default:
            CPPUNIT_FAIL("Unspecified Sieve node type");
            return CPN::shared_ptr<CPN::NodeBase>();
        }
    }

    static inline void RegisterFilter() {
        CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory>(new SieveNodeFactory(FILTER_TYPE, FILTER)));
    }
    static inline void RegisterProducer() {
        CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory>(new SieveNodeFactory(PRODUCER_TYPE, PRODUCER)));
    }
    static inline void RegisterResult() {
        CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory>(new SieveNodeFactory(RESULT_TYPE, RESULT)));
    }
private:
    Type type;
};


void SieveTest::setUp(void) {
    SieveNodeFactory::RegisterFilter();
    SieveNodeFactory::RegisterProducer();
    SieveNodeFactory::RegisterResult();
    kernel = new CPN::Kernel(CPN::KernelAttr("Testing"));
}

void SieveTest::tearDown(void) {
    delete kernel;
    kernel = 0;
}


void SieveTest::RunTest(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    AutoBuffer buffer(NUMPRIMES*sizeof(unsigned long));
    CPPUNIT_ASSERT(buffer.GetBuffer());
    CPN::NodeAttr nattr("TheProducer", PRODUCER_TYPE);
    kernel->CreateNode(nattr);

    SieveNumber nodenum = 0;
    std::string filtername = ToString(FILTER_FORMAT, nodenum);
    nattr.SetName(filtername);
    nattr.SetTypeName(FILTER_TYPE);
    nattr.SetParam(StaticBuffer(&nodenum, sizeof(SieveNumber)));
    kernel->CreateNode(nattr);

    nattr.SetName("TheResult");
    nattr.SetTypeName(RESULT_TYPE);
    SieveResultNode::Param resultparam = { (SieveNumber*)buffer.GetBuffer(), buffer.GetSize() };
    nattr.SetParam(StaticBuffer(&resultparam, sizeof(resultparam)));
    kernel->CreateNode(nattr);

    CPN::QueueAttr qattr(100, 100);
    qattr.SetReader(filtername, PORT_IN);
    qattr.SetWriter("TheProducer", PORT_OUT);
    kernel->CreateQueue(qattr);

    std::string portname = ToString(PORT_FORMAT, nodenum);
    qattr.SetWriter(filtername, portname);
    qattr.SetReader("TheResult", portname);
    kernel->CreateQueue(qattr);

    kernel->WaitNodeTerminate("TheResult");
    unsigned long* result = (unsigned long*) buffer.GetBuffer();
    for (unsigned long i = 0; i < NUMPRIMES; i++) {
        CPPUNIT_ASSERT_EQUAL(result[i], PRIMES[i]);
    }
}


