
#pragma once
#include "Executor.h"
#include "BlockingQueue.h"
#include "Pthread.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <deque>
#include <set>
namespace Sync {

    /**
     * A very simple thread pool
     */
    class ThreadPool : public Executor {
    public:
        ThreadPool();
        ThreadPool(unsigned minthreads_, unsigned maxthreads_, double timeout_);
        ~ThreadPool();

        /**
         * Wait for the queue to become empty.
         */
        void Wait();
        /**
         * Wait for all tasks to be complete and then shutdown.
         * Any leftover tasks may be executed in the calling thread.
         */
        void Shutdown();
        using Executor::Execute;
        void Execute(shared_ptr<Runnable> r);
    private:
    
        PthreadMutex lock;
        PthreadCondition enqueue_cond;
        PthreadCondition dequeue_cond;
        bool shutdown;
        typedef std::deque<shared_ptr<Runnable> > TaskQueue;
        TaskQueue taskqueue;
        typedef std::set<shared_ptr<Pthread> > ThreadSet;
        ThreadSet threads;
        unsigned minthreads;
        unsigned maxthreads;
        unsigned numthreads;
        unsigned numtasks;
        unsigned waiting;
        double timeout;
        bool needscleanup;

        void Spawn();
        void InternalCleanup();
        void *TaskThread();
    };
}

