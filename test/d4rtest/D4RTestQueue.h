#pragma once

#include "D4RQueue.h"
#include "Variant.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "Logger.h"
#include <exception>
#include <string>

namespace D4R {
    class TestQueue : public D4R::QueueBase, public Logger {
    public:
        TestQueue(unsigned initialsize, const std::string &name_);
        TestQueue(const Variant &queued);

        void Enqueue(unsigned amount);
        void Dequeue(unsigned amount);

        void Abort();
    protected:
        bool WriteBlocked();

        bool ReadBlocked();

    public:
        unsigned Freespace() {
            PthreadMutexProtected al(lock);
            return queuesize - count;
        }

        unsigned Count() {
            PthreadMutexProtected al(lock);
            return count;
        }
        unsigned QueueSize() {
            PthreadMutexProtected al(lock);
            return queuesize;
        }

        const std::string &GetName() const { return name; }

        void Lock() const { lock.Lock(); }

        void Unlock() const { lock.Unlock(); }

        bool Detected();
    private:

        void Detect();

        void Signal() { cond.Broadcast(); }
        void Wait() { cond.Wait(lock); }

        mutable PthreadMutex lock;
        PthreadCondition cond;
        bool detected;
        bool aborted;
        unsigned queuesize;
        unsigned count;
        unsigned enqueue_amount;
        unsigned dequeue_amount;
        const std::string name;
    };

    class TestQueueAbortException : public std::exception {
    public:
        TestQueueAbortException(const std::string &name) throw() {
            msg = "TestQueue " + name + " aborted.";
        }
        ~TestQueueAbortException() throw() {}
        const char *what() const throw() { return msg.c_str(); }
        std::string msg;
    };
}
