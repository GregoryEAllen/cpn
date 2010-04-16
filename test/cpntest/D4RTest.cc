
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
#include "ToString.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"

#include <stdio.h>

#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( D4RTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

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
        JSONToVariant parse;
        parse.Parse(attr.GetParam().data(), attr.GetParam().size());
        ASSERT(parse.Done());
        Variant noded = parse.Get();
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
        QueueWriterAdapter<unsigned> out = GetWriter(qname);
        Trace("Enqueue %s(%llu): %u", qname.c_str(), out.GetKey(), amount);
#if 0
        std::vector<unsigned> buf;
        for (unsigned i = 0; i < amount; ++i) {
            buf.push_back(i);
        }
        out.Enqueue(&buf[0], amount);
#else
        for (unsigned i = 0; i < amount; ++i) {
            out.Enqueue(&i, 1);
        }
#endif
    }

    void Dequeue(const std::string &qname, unsigned amount) {
        QueueReaderAdapter<unsigned> in = GetReader(qname);
        Trace("Dequeue %s(%llu): %u", qname.c_str(), in.GetKey(), amount);
#if 0
        std::vector<unsigned> buf(amount);
        in.Dequeue(&buf[0], amount);
#else
        unsigned bogus;
        for (unsigned i = 0; i < amount; ++i) {
            in.Dequeue(&bogus, 1);
        }
#endif
    }

    void VerifyReaderSize(const std::string &qname, unsigned amount) {
        QueueReaderAdapter<unsigned> in = GetReader(qname);
        Trace("%s %u ?= %u", qname.c_str(), in.QueueLength(), amount);
        if (in.QueueLength() != amount) {
            testerbase->Failure(this, "Queuesize is not expected size");
        }
    }

    void VerifyWriterSize(const std::string &qname, unsigned amount) {
        QueueWriterAdapter<unsigned> out = GetWriter(qname);
        Trace("%s %u ?= %u", qname.c_str(), out.QueueLength(), amount);
        if (out.QueueLength() != amount) {
            testerbase->Failure(this, "Queuesize is not expected size");
        }
    }
};

CPN_DECLARE_NODE_FACTORY(D4RTestNodeType, TestNode);

void D4RTest::setUp() {
    PthreadMutexProtected al(lock);
    successes = 0;
}

void D4RTest::tearDown() {
}

void D4RTest::RunTest(int numkernels) {
    std::string ext = ".test";
    Directory dir("D4R/Tests");
    unsigned runs = 0;
    unsigned failures = 0;

    for (;!dir.End(); dir.Next()) {
        if (!dir.IsRegularFile()) continue;
        std::string fpath = dir.BaseName();
        if (fpath[0] == '.' || fpath[0] == '_') continue;
        if (fpath.size() < ext.size() ||
                fpath.substr(fpath.size() - ext.size(), ext.size()) != ext) {
            continue; 
        }
        fpath = dir.FullName();
        std::vector<char> buf(4096);

        DEBUG("Processing %s\n", fpath.c_str());
        FILE *f = fopen(fpath.c_str(), "r");
        if (!f) {
            perror("Could not open file");
            continue;
        }
        JSONToVariant parse;
        while (!feof(f)) {
            unsigned numread = fread(&buf[0], 1, buf.size(), f);
            parse.Parse(&buf[0], numread);
            if (parse.Error()) {
                printf("Unabled to parse line: %u column: %u\n", parse.GetLine(), parse.GetColumn());
                break;
            }
        }
        fclose(f);

        CPPUNIT_ASSERT(parse.Done());

        Variant conf = parse.Get();
        database = Database::Local();
        database->LogLevel(Logger::WARNING);
        database->UseD4R(true);
        database->SwallowBrokenQueueExceptions(true);
        database->GrowQueueMaxThreshold(false);
        Output(database.get());
        LogLevel(database->LogLevel());
        for (int i = 0; i < numkernels; ++i) {
            kernels.push_back(
                    new Kernel(KernelAttr(ToString("K %d", i)).SetDatabase(database))
                    );
        }

        DEBUG("Starting test %s with %d kernels\n", dir.BaseName().c_str(), numkernels);
        success = true;
        try {
            Setup(conf);
        } catch (const CPN::ShutdownException &e) {
        }

        database->WaitForAllNodeEnd();
        while (!kernels.empty()) {
            delete kernels.back();
            kernels.pop_back();
        }
        runs++;
        DEBUG("Test %s : %s\n", dir.BaseName().c_str(), success ? "success" : "failure");
        database.reset();
        if (!success) {
            failures++;
            printf("*************** Failure! ******************\n");
        }
    }
    CPPUNIT_ASSERT(failures == 0);
}

void D4RTest::RunOneKernelTest() {
    RunTest(1);
}

void D4RTest::RunTwoKernelTest() {
    RunTest(2);
}

void D4RTest::Deadlock(TestNodeBase *tnb) {
    NodeBase *n = dynamic_cast<NodeBase*>(tnb);
    Info("%s detected deadlock correctly", n->GetName().c_str());
    {
        PthreadMutexProtected al(lock);
        successes++;
    }
    database->Terminate();
}

void D4RTest::Failure(TestNodeBase *tnb, const std::string &msg) {
    NodeBase *n = dynamic_cast<NodeBase*>(tnb);
    Error("%s: %s", n->GetName().c_str(), msg.c_str());
    D4R::TesterBase::Failure(tnb, msg);
}

void D4RTest::Complete(TestNodeBase *tnb) {
    NodeBase *n = dynamic_cast<NodeBase*>(tnb);
    Info("%s completed correctly", n->GetName().c_str());
    PthreadMutexProtected al(lock);
    successes++;
}

void D4RTest::CreateNode(const Variant &noded) {
    NodeAttr attr(noded["name"].AsString(), TESTNODETYPE);
    attr.SetParam(VariantToJSON(noded));
    TesterBase *tb = this;
    attr.SetParam(StaticConstBuffer(&tb, sizeof(tb)));
    kernels[noded["key"].AsInt() % kernels.size()]->CreateNode(attr);
}

void D4RTest::CreateQueue(const Variant &queued) {
    QueueAttr attr(queued["size"].AsUnsigned() * sizeof(unsigned), sizeof(unsigned));
    attr.SetDatatype<unsigned>();
    attr.SetReader(queued["reader"].AsString(), queued["name"].AsString());
    attr.SetWriter(queued["writer"].AsString(), queued["name"].AsString());
    kernels.front()->CreateQueue(attr);
}

void D4RTest::Abort() {
    database->Terminate();
}

