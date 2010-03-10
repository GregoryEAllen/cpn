
#pragma once

#include "D4RNode.h"
#include "Logger.h"
#include "Variant.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
namespace D4R {

    class TesterBase;

    class TestNodeBase : public Logger {
    public:
        // do an enqueue with the given queue
        static const char OP_ENQUEUE[];
        // do a dequeue with the given queue
        static const char OP_DEQUEUE[];
        // verify that the given queue is the specified size
        static const char OP_VERIFY_READER_SIZE[];
        static const char OP_VERIFY_WRITER_SIZE[];
        // tag instruction saying that a deadlock should have occured
        // on the previous instruction
        static const char OP_VERIFY_DEADLOCK[];
        // done exit normally
        static const char OP_EXIT[];

        TestNodeBase(TesterBase *tb);

        virtual ~TestNodeBase();

        void Run();

        virtual const std::string &GetName() const = 0;
        // all opcodes must be added before run starts.
        void AddOp(const Variant &op);
        void AddOp(const std::string &opcode, const std::string &qname, unsigned amount);

        void AddDequeue(const std::string &qname, unsigned amount) {
            AddOp(OP_DEQUEUE, qname, amount);
        }

        void AddEnqueue(const std::string &qname, unsigned amount) {
            AddOp(OP_ENQUEUE, qname, amount);
        }

        void AddVerifyReaderSize(const std::string &qname, unsigned amount) {
            AddOp(OP_VERIFY_READER_SIZE, qname, amount);
        }

        void AddVerifyWriterSize(const std::string &qname, unsigned amount) {
            AddOp(OP_VERIFY_WRITER_SIZE, qname, amount);
        }

        void AddVerifyDeadlock() {
            AddOp(OP_VERIFY_DEADLOCK, "", 0);
        }

        void AddExit() {
            AddOp(OP_EXIT, "", 0);
        }

    protected:
        virtual void Enqueue(const std::string &qname, unsigned amount) = 0;
        virtual void Dequeue(const std::string &qname, unsigned amount) = 0;
        virtual void VerifyReaderSize(const std::string &qname, unsigned amount) = 0;
        virtual void VerifyWriterSize(const std::string &qname, unsigned amount) = 0;

        typedef std::deque<Variant> OpcodeQueue;
        OpcodeQueue opqueue;
        TesterBase *testerbase;
        PthreadMutex lock;
        PthreadCondition cond;
    };
}

