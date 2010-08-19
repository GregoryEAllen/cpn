
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

        void Shutdown();
        using Executor::Execute;
        void Execute(shared_ptr<Runnable> r);
    private:
    
        PthreadMutex lock;
        PthreadCondition cond;
        bool shutdown;
        typedef std::deque<shared_ptr<Runnable> > TaskQueue;
        TaskQueue taskqueue;
        typedef std::set<shared_ptr<Pthread> > ThreadSet;
        ThreadSet threads;
        unsigned minthreads;
        unsigned maxthreads;
        unsigned numthreads;
        unsigned waiting;
        double timeout;

        void Spawn();
        void InternalCleanup();
        void Cleanup();
        void SpawnCleanup();
        void *TaskThread();
    };
}

