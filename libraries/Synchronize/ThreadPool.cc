
#include "ThreadPool.h"
#include "SysConf.h"
#include "PthreadFunctional.h"
#include "Assert.h"
#include <unistd.h>

namespace Sync {

    ThreadPool::ThreadPool(unsigned minthreads_, unsigned maxthreads_, double timeout_)
        : shutdown(false), minthreads(minthreads_), maxthreads(maxthreads_),
        numthreads(0), waiting(0), timeout(timeout_)
    {
    }
    
    ThreadPool::ThreadPool()
        : shutdown(false), minthreads(1), maxthreads(2*NumProcessorsOnline()),
        numthreads(0), waiting(0), timeout(60)
    {
    }

    ThreadPool::~ThreadPool() {
        Shutdown();
    }

    void ThreadPool::Spawn() {
        if (shutdown) return;
        shared_ptr<Pthread> thread = shared_ptr<Pthread>(CreatePthreadFunctional(this, &ThreadPool::TaskThread));
        threads.insert(thread);
        thread->Start();
        ++numthreads;
    }

    void ThreadPool::Shutdown() {
        AutoLock<PthreadMutex> al(lock);
        shutdown = true;
        cond.Broadcast();
    }

    void ThreadPool::Execute(shared_ptr<Runnable> r) {
        AutoLock<PthreadMutex> al(lock);
        ASSERT(!shutdown, "Cannot execute on a shutdown threadpool");
        taskqueue.push_back(r);
        InternalCleanup();
        if (waiting == 0 && numthreads < maxthreads) {
            Spawn();
        }
        cond.Signal();
    }

    void ThreadPool::InternalCleanup() {
        ThreadSet::iterator cur, end;
        cur = threads.begin();
        end = threads.end();
        while (cur != end) {
            if ((*cur)->Done()) {
                threads.erase(cur++);
            } else {
                ++cur;
            }
        }
    }

    void ThreadPool::Cleanup() {
        AutoLock<PthreadMutex> al(lock);
        InternalCleanup();
    }

    void ThreadPool::SpawnCleanup() {
        std::auto_ptr<RunnableFuture<void> > clean = CreateRunnableFuture(this, &ThreadPool::Cleanup);
        taskqueue.push_back(shared_ptr<Runnable>(clean));
        cond.Signal();
    }

    void *ThreadPool::TaskThread() {
        while (true) {
            AutoLock<PthreadMutex> al(lock);
            while (taskqueue.empty()) {
                bool timedout = false;
                bool die = false;
                ++waiting;
                if (timeout > 0) {
                    timedout = cond.TimedWait(lock, timeout);
                } else {
                    cond.Wait(lock);
                }
                --waiting;
                if (shutdown) die = true;
                if (timedout && taskqueue.empty()) {
                    if (numthreads > minthreads) {
                        SpawnCleanup();
                        die = true;
                    }
                }
                if (die) {
                    --numthreads;
                    return 0;
                }
            }
            shared_ptr<Runnable> r = taskqueue.front();
            taskqueue.pop_front();
            al.Unlock();
            if (r) r->Run();
        }
    }

}
