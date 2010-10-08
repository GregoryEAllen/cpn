
#include "SieveTest.h"
#include "Kernel.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "NodeAttr.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "Database.h"
#include "ToString.h"
#include "Variant.h"
#include "VariantToJSON.h"
#include "JSONToVariant.h"
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

typedef unsigned long long SieveNumber;

const SieveNumber PRIMES[] = {
2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
};

const SieveNumber NUMPRIMES = sizeof(PRIMES)/sizeof(SieveNumber);

const SieveNumber MAX_PRIME_VALUE = 100;

const char PORT_IN[] = "x";
const char PORT_OUT[] = "y";
const char PORT_RESULT[] = "res";
const char PORT_FORMAT[] = "Result: %llu";
const char FILTER_FORMAT[] = "Filter: %llu";
const char QUEUE_FORMAT[] = "Queue %llu";

class SieveResultNode : public CPN::NodeBase {
public:
    SieveResultNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
    void Process();
    void CreateNextFilter(SieveNumber filternum);
private:
    std::vector<std::string> hosts;
};

CPN_DECLARE_NODE_FACTORY(SieveResultNode, SieveResultNode);

SieveResultNode::SieveResultNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr)
{
    if (!attr.GetParam().empty()) {
        JSONToVariant p;
        p.Parse(attr.GetParam());
        ASSERT(p.Done());
        Variant param = p.Get();
        for (Variant::ListIterator i = param.ListBegin(), e = param.ListEnd();
                i != e; ++i) {
            hosts.push_back(i->AsString());
        }
    }
}

void SieveResultNode::Process() {
    std::string ourname = GetName();
    SieveNumber portnum = 0;
    CPN::QueueReaderAdapter<SieveNumber> in = GetReader(ToString(PORT_FORMAT, portnum));
    CPN::QueueWriterAdapter<SieveNumber> out = GetWriter(PORT_RESULT);
    DBPRINT("Result node %s started (in %llu)\n", ourname.c_str(), in.GetKey());
    SieveNumber index = 0;
    while (index < NUMPRIMES) {
        SieveNumber value;
        ENSURE(in.Dequeue(&value, 1));
        if (value == 0) {
            ++portnum;
            CreateNextFilter(portnum);
            in.Release();
            in = GetReader(ToString(PORT_FORMAT, portnum));
            DBPRINT("Result swapped port to %llu (%llu)\n", portnum, in.GetKey());
        } else {
            out.Enqueue(&value, 1);
            DBPRINT("Result[%llu] = %llu\n", index, value);
            ++index;
        }
    }
    SieveNumber val = -1;
    ENSURE(in.Dequeue(&val, 1));
    ASSERT(0 == val);
    in.Release();
    DBPRINT("%s stopped\n", ourname.c_str());
}

void SieveResultNode::CreateNextFilter(SieveNumber filternum) {
    std::string nodename = ToString(FILTER_FORMAT, filternum); 
    CPN::NodeAttr nattr(nodename, "SieveFilterNode");
    if (!hosts.empty()) {
        nattr.SetHost(hosts[filternum%hosts.size()]);
    }
    kernel.CreateNode(nattr);
    // A queue between us and next
    CPN::QueueAttr qattr(100, 100);
    qattr.SetDatatype<SieveNumber>();
    qattr.SetReader(nodename, PORT_IN);
    qattr.SetWriter(ToString(FILTER_FORMAT, filternum - 1), PORT_OUT);
    kernel.CreateQueue(qattr);
    // A queue between next and result
    std::string portname = ToString(PORT_FORMAT, filternum);
    qattr.SetWriter(nodename, PORT_RESULT);
    qattr.SetReader("TheResult", portname);
    kernel.CreateQueue(qattr);
}

class SieveFilterNode : public CPN::NodeBase {
public:
    SieveFilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
    void Process();
private:
};

CPN_DECLARE_NODE_FACTORY(SieveFilterNode, SieveFilterNode);

SieveFilterNode::SieveFilterNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr) {
}

void SieveFilterNode::Process() {
    std::string ourname = GetName();
    CPN::QueueReaderAdapter<SieveNumber> in = GetReader(PORT_IN);
    CPN::QueueWriterAdapter<SieveNumber> result = GetWriter(PORT_RESULT);
    CPN::QueueWriterAdapter<SieveNumber> out;
    DBPRINT("Filter node %s started (in %llu result %llu)\n", ourname.c_str(), in.GetKey(), result.GetKey());
    SieveNumber input = 0;
    SieveNumber value = 0;
    ENSURE(in.Dequeue(&input, 1));
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
        out = GetWriter(PORT_OUT);
        DBPRINT("%s swapped to out port (%llu)\n", ourname.c_str(), out.GetKey());
    } else {
        out = result;
    }
    while (true) {
        ENSURE(in.Dequeue(&input, 1));
        if (input%value != 0) {
            out.Enqueue(&input, 1);
        }
        if (input == 0) {
            out.Enqueue(&input, 1);
            break;
        }
    }
    in.Release();
    out.Release();
    DBPRINT("%s stopped\n", ourname.c_str());
}

class SieveProducerNode : public CPN::NodeBase {
public:
    SieveProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr);
    void Process(void);
};

CPN_DECLARE_NODE_FACTORY(SieveProducerNode, SieveProducerNode);

SieveProducerNode::SieveProducerNode(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr) {
}

void SieveProducerNode::Process(void) {
    std::string ourname = GetName();
    CPN::QueueWriterAdapter<SieveNumber> out = GetWriter(PORT_OUT);
    DBPRINT("Producer node %s started (out %llu)\n", ourname.c_str(), out.GetKey());
    SieveNumber index = 2;
    while (index <= MAX_PRIME_VALUE) {
        DBPRINT("Enqueueing %llu\n", index);
        out.Enqueue(&index, 1);
        ++index;
    }
    index = 0;
    out.Enqueue(&index, 1);
    out.Release();
    DBPRINT("%s stopped\n", ourname.c_str());
}

void SieveTest::setUp(void) {
}

void SieveTest::tearDown(void) {
}


void SieveTest::RunTest(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(CPN::KernelAttr("Testing"));
    kernel.GetDatabase()->UseD4R(false);
    std::vector<SieveNumber> result(NUMPRIMES);

    CPN::NodeAttr nattr("TheProducer", "SieveProducerNode");
    kernel.CreateNode(nattr);

    SieveNumber nodenum = 0;
    std::string filtername = ToString(FILTER_FORMAT, nodenum);
    nattr.SetName(filtername);
    nattr.SetTypeName("SieveFilterNode");
    kernel.CreateNode(nattr);

    nattr.SetName("TheResult");
    nattr.SetTypeName("SieveResultNode");
    kernel.CreateNode(nattr);

    CPN::QueueAttr qattr(100, 100);
    qattr.SetDatatype<SieveNumber>();
    qattr.SetReader(filtername, PORT_IN);
    qattr.SetWriter("TheProducer", PORT_OUT);
    kernel.CreateQueue(qattr);

    std::string portname = ToString(PORT_FORMAT, nodenum);
    qattr.SetWriter(filtername, PORT_RESULT);
    qattr.SetReader("TheResult", portname);
    kernel.CreateQueue(qattr);
    CPN::Key_t pseudokey = kernel.CreatePseudoNode("output");
    qattr.SetWriter("TheResult", PORT_RESULT);
    qattr.SetReader("output", PORT_RESULT);
    kernel.CreateQueue(qattr);
    CPN::QueueReaderAdapter<SieveNumber> in = kernel.GetPseudoReader(pseudokey, PORT_RESULT);
    in.Dequeue(&result[0], result.size());
    in.Release();
    kernel.DestroyPseudoNode(pseudokey);

    kernel.WaitNodeTerminate("TheResult");
    for (SieveNumber i = 0; i < NUMPRIMES; i++) {
        CPPUNIT_ASSERT_EQUAL(result[i], PRIMES[i]);
    }
}

void SieveTest::RunTwoKernelTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    std::vector<SieveNumber> result(NUMPRIMES);
    CPN::shared_ptr<CPN::Database> database = CPN::Database::Local();
    database->LogLevel(Logger::WARNING);
    database->UseD4R(false);
    CPN::Kernel kone(CPN::KernelAttr("one").SetDatabase(database).SetRemoteEnabled(true));
    CPN::Kernel ktwo(CPN::KernelAttr("two").SetDatabase(database).SetRemoteEnabled(true));
    Variant hosts;
    hosts.Append("one");
    hosts.Append("two");

    CPN::NodeAttr nattr("TheProducer", "SieveProducerNode");
    kone.CreateNode(nattr);

    SieveNumber nodenum = 0;
    std::string filtername = ToString(FILTER_FORMAT, nodenum);
    nattr.SetName(filtername);
    nattr.SetTypeName("SieveFilterNode");
    ktwo.CreateNode(nattr);

    nattr.SetName("TheResult");
    nattr.SetTypeName("SieveResultNode");
    nattr.SetParam(VariantToJSON(hosts));
    kone.CreateNode(nattr);

    CPN::QueueAttr qattr(100, 100);
    qattr.SetDatatype<SieveNumber>();
    qattr.SetReader(filtername, PORT_IN);
    qattr.SetWriter("TheProducer", PORT_OUT);
    ktwo.CreateQueue(qattr);

    std::string portname = ToString(PORT_FORMAT, nodenum);
    qattr.SetWriter(filtername, PORT_RESULT);
    qattr.SetReader("TheResult", portname);
    kone.CreateQueue(qattr);

    CPN::Key_t pseudokey = kone.CreatePseudoNode("output");
    qattr.SetWriter("TheResult", PORT_RESULT);
    qattr.SetReader("output", PORT_RESULT);
    kone.CreateQueue(qattr);
    CPN::QueueReaderAdapter<SieveNumber> in = kone.GetPseudoReader(pseudokey, PORT_RESULT);
    in.Dequeue(&result[0], result.size());
    in.Release();
    kone.DestroyPseudoNode(pseudokey);

    kone.WaitNodeTerminate("TheResult");
    for (SieveNumber i = 0; i < NUMPRIMES; i++) {
        CPPUNIT_ASSERT_EQUAL(result[i], PRIMES[i]);
    }
}

