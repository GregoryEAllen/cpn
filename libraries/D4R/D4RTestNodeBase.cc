
#include "D4RTestNodeBase.h"
#include "D4RTesterBase.h"
#include "D4RDeadlockException.h"
#include "Assert.h"

namespace D4R {

    const char TestNodeBase::OP_ENQUEUE[] = "enqueue";
    const char TestNodeBase::OP_DEQUEUE[] = "dequeue";
    const char TestNodeBase::OP_VERIFY_READER_SIZE[] = "verify reader size";
    const char TestNodeBase::OP_VERIFY_WRITER_SIZE[] = "verify writer size";
    const char TestNodeBase::OP_VERIFY_DEADLOCK[] = "verify deadlock";
    const char TestNodeBase::OP_EXIT[] = "exit";

    TestNodeBase::TestNodeBase(TesterBase *tb)
        : Logger(), testerbase(tb)
    {}

    TestNodeBase::~TestNodeBase() {}

    void TestNodeBase::AddOp(const Variant &op) {
        PthreadMutexProtected al(lock);
        opqueue.push_back(op);
        cond.Signal();
    }

    void TestNodeBase::AddOp(const std::string &opcode, const std::string &qname, unsigned amount) {
        AddOp(Variant().Append(opcode).Append(qname).Append(amount));
    }

    void TestNodeBase::Run() {
        try {
            bool loop = true;
            while (loop) {
                Variant op;
                {
                    PthreadMutexProtected al(lock);
                    while (opqueue.empty()) {
                        cond.Wait(lock);
                    }
                    op = opqueue.front();
                    opqueue.pop_front();
                }
                std::string opname = op[0].AsString();
                if (opname == OP_ENQUEUE) {
                    Enqueue(op[1].AsString(), op[2].AsUnsigned());
                } else if (opname == OP_DEQUEUE) {
                    Dequeue(op[1].AsString(), op[2].AsUnsigned());
                } else if (opname == OP_VERIFY_READER_SIZE) {
                    VerifyReaderSize(op[1].AsString(), op[2].AsUnsigned());
                } else if (opname == OP_VERIFY_WRITER_SIZE) {
                    VerifyWriterSize(op[1].AsString(), op[2].AsUnsigned());
                } else if (opname == OP_EXIT) {
                    loop = false;
                } else if (opname == OP_VERIFY_DEADLOCK) {
                    testerbase->Failure(this, "No deadlock detected");
                    loop = false;
                } else {
                    testerbase->Failure(this, "Unknown op: " + opname);
                    loop = false;
                }
            }
        } catch (const DeadlockException &e) {
                Variant op;
                op = opqueue.front();
                opqueue.pop_front();
                std::string opname = op[0].AsString();
                if (opname == OP_VERIFY_DEADLOCK) {
                    Debug("Deadlock detected");
                    testerbase->Deadlock(this);
                } else {
                    testerbase->Failure(this, "Deadlock exception but no deadlock expected");
                }
                throw;
        } catch (const AssertException &e) {
            testerbase->Failure(this, e.what());
            throw;
        }
        testerbase->Complete(this);
    }

}

