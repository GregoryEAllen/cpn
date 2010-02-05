
#include "D4RTest.h"
#include "D4RTestNodeBase.h"
#include "Database.h"
#include "Kernel.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "Directory.h"
#include "Variant.h"

#include <stdio.h>

#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( D4RTest );

using CPN::NodeBase;
using CPN::Kernel;
using CPN::Database;
using CPN::QueueWriterAdapter;
using CPN::QueueReaderAdapter;
using CPN::shared_ptr;
using CPN::KernelAttr;
using CPN::NodeAttr;
using CPN::QueueAttr;
using D4R::TestNodeBase;
using D4R::TesterBase;

#define TESTNODETYPE "D4RTestNodeType"

class TestNode : public NodeBase, public TestNodeBase {
public:
    TestNode(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr),
        TestNodeBase(*reinterpret_cast<TesterBase**>(const_cast<void*>(attr.GetArg().GetBuffer())))
    {
        Logger::Name(NodeBase::GetName());
        Variant noded = Variant::FromJSON(attr.GetParam());
        Variant::ConstListIterator itr = noded["instructions"].ListBegin();
        while (itr != noded["instructions"].ListEnd()) {
            AddOp(*itr);
            ++itr;
        }
    }
    const std::string &GetName() const { return NodeBase::GetName(); }

    void Process() {
        Output(testerbase);
        LogLevel(testerbase->LogLevel());
        Run();
    }

    void Enqueue(const std::string &qname, unsigned amount) {
        Trace("Enqueue %s: %u", qname.c_str(), amount);
        QueueWriterAdapter<unsigned> out = GetWriter(qname);
        std::vector<unsigned> buf;
        for (unsigned i = 0; i < amount; ++i) {
            buf.push_back(i);
        }
        out.Enqueue(&buf[0], amount);
    }

    void Dequeue(const std::string &qname, unsigned amount) {
        Trace("Dequeue %s: %u", qname.c_str(), amount);
        QueueReaderAdapter<unsigned> in = GetReader(qname);
        std::vector<unsigned> buf(amount);
        in.Dequeue(&buf[0], amount);
    }

    void VerifyReaderSize(const std::string &qname, unsigned amount) {
        QueueReaderAdapter<unsigned> in = GetReader(qname);
        ASSERT(in.QueueLength() == amount, "%u != %u", in.QueueLength(), amount);
    }

    void VerifyWriterSize(const std::string &qname, unsigned amount) {
        QueueWriterAdapter<unsigned> out = GetWriter(qname);
        ASSERT(out.QueueLength() == amount, "%u != %u", out.QueueLength(), amount);
    }
};

class TestNodeFactory : public CPN::NodeFactory {
public:
    TestNodeFactory() : CPN::NodeFactory(TESTNODETYPE) {}
    shared_ptr<NodeBase> Create(Kernel &ker, const NodeAttr &attr) {
        return shared_ptr<NodeBase>(new TestNode(ker, attr));
    }
};

extern "C" shared_ptr<CPN::NodeFactory> cpninitD4RTestNodeType(void);

shared_ptr<CPN::NodeFactory> cpninitD4RTestNodeType(void) {
    return shared_ptr<CPN::NodeFactory>(new TestNodeFactory);
}


void D4RTest::setUp() {
    successes = 0;
}

void D4RTest::tearDown() {
}

void D4RTest::RunTest() {
    std::string ext = ".test";
    Directory dir("D4R/Tests");
    unsigned runs = 0;

    for (;!dir.End(); dir.Next()) {
        if (!dir.IsRegularFile()) continue;
        std::string fpath = dir.BaseName();
        if (fpath[0] == '.' || fpath[0] == '_') continue;
        if (fpath.size() < ext.size() ||
                fpath.substr(fpath.size() - ext.size(), ext.size()) != ext) {
            continue; 
        }
        fpath = dir.FullName();
        std::vector<char> buf(dir.Size());

        printf("Processing %s\n", fpath.c_str());
        FILE *f = fopen(fpath.c_str(), "r");
        if (!f) {
            perror("Could not open file");
            continue;
        }
        if (fread(&buf[0], 1, buf.size(), f) != buf.size()) {
            printf("Unable to read file\n");
            continue;
        }
        fclose(f);

        Variant conf = Variant::FromJSON(buf);

        shared_ptr<Database> database = Database::Local();
        database->LogLevel(Logger::WARNING);
        database->UseD4R(true);
        database->SwallowBrokenQueueExceptions(true);
        Output(database.get());
        LogLevel(database->LogLevel());
        kernel = new Kernel(KernelAttr("testkernel").SetDatabase(database));

        Debug("Starting test %s", dir.BaseName().c_str());
        success = false;
        Setup(conf);

        database->WaitForAllNodeEnd();
        delete kernel;
        kernel = 0;
        runs++;
        Debug("Test %s : %s", dir.BaseName().c_str(), success ? "success" : "failure");
        CPPUNIT_ASSERT(success);
    }
}

void D4RTest::Deadlock(TestNodeBase *tnb) {
    NodeBase *n = dynamic_cast<NodeBase*>(tnb);
    Info("%s detected deadlock correctly", n->GetName().c_str());
    successes++;
    success = true;
    kernel->Terminate();
}

void D4RTest::Failure(TestNodeBase *tnb, const std::string &msg) {
    NodeBase *n = dynamic_cast<NodeBase*>(tnb);
    Error("%s: %s", n->GetName().c_str(), msg.c_str());
}

void D4RTest::Complete(TestNodeBase *tnb) {
    NodeBase *n = dynamic_cast<NodeBase*>(tnb);
    Info("%s completed correctly", n->GetName().c_str());
    successes++;
    success = true;
}

void D4RTest::CreateNode(const Variant &noded) {
    NodeAttr attr(noded["name"].AsString(), TESTNODETYPE);
    attr.SetParam(noded.AsJSON());
    TesterBase *tb = this;
    attr.SetParam(StaticConstBuffer(&tb, sizeof(tb)));
    kernel->CreateNode(attr);
}

void D4RTest::CreateQueue(const Variant &queued) {
    QueueAttr attr(queued["size"].AsUnsigned() * sizeof(unsigned), sizeof(unsigned));
    attr.SetDatatype<unsigned>();
    attr.SetReader(queued["reader"].AsString(), queued["name"].AsString());
    attr.SetWriter(queued["writer"].AsString(), queued["name"].AsString());
    kernel->CreateQueue(attr);
}


