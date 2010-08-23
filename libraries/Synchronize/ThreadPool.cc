
#include "ThreadPool.h"
#include "SysConf.h"
#include "PthreadFunctional.h"
#include "Assert.h"
#include <unistd.h>

namespace Sync {

    ThreadPool::ThreadPool(unsigned minthreads_, unsigned maxthreads_, double timeout_)
        : shutdown(false), minthreads(minthreads_), maxthreads(maxthreads_),
        numthreads(0), numtasks(0), waiting(0), timeout(timeout_), needscleanup(false)
    {
        if (minthreads > maxthreads) maxthreads = minthreads;
    }
    
    ThreadPool::ThreadPool()
        : shutdown(false), minthreads(0), maxthreads(2*NumProcessorsOnline()),
        numthreads(0), numtasks(0), waiting(0), timeout(60)
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

    void ThreadPool::Wait() {
        AutoLock<PthreadMutex> al(lock);
        if (shutdown) return;
        while (numtasks != 0 && numthreads != 0) {
            dequeue_cond.Wait(lock);
        }
    }

    void ThreadPool::Shutdown() {
        AutoLock<PthreadMutex> al(lock);
        while (numtasks != 0 && numthreads != 0) {
            dequeue_cond.Wait(lock);
        }
        shutdown = true;
        dequeue_cond.Broadcast();
        enqueue_cond.Broadcast();
        ThreadSet threadcopy = threads;
        al.Unlock();
        ThreadSet::iterator cur = threadcopy.begin(), end = threadcopy.end();
        while (cur != end) {
            (*cur)->Join();
            ++cur;
        }
        // There are no other threads touching our insides now.
        // finish up
        threadcopy.clear();
        threads.clear();
        TaskQueue::iterator tqcur = taskqueue.begin(), tqend = taskqueue.end();
        while (tqcur != tqend) {
            (*tqcur)->Run();
            tqcur++;
        }
        taskqueue.clear();
    }

    void ThreadPool::Execute(shared_ptr<Runnable> r) {
        AutoLock<PthreadMutex> al(lock);
        ASSERT(!shutdown, "Cannot execute on a shutdown threadpool");
        taskqueue.push_back(r);
        ++numtasks;
        InternalCleanup();
        if (waiting == 0 && numthreads < maxthreads) {
            Spawn();
        }
        enqueue_cond.Signal();
    }

    void ThreadPool::InternalCleanup() {
        if (needscleanup) {
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
            needscleanup = false;
        }
    }

    void *ThreadPool::TaskThread() {
        AutoLock<PthreadMutex> al(lock);
        while (true) {
            bool timedout = false;
            while (taskqueue.empty()) {
                bool die = false;
                if (shutdown) die = true;
                if (timedout && (numthreads > minthreads)) {
                    die = true;
                }
                if (die) {
                    --numthreads;
                    needscleanup = true;
                    enqueue_cond.Signal();
                    dequeue_cond.Signal();
                    return 0;
                }
                InternalCleanup();
                ++waiting;
                if (timeout > 0) {
                    timedout = enqueue_cond.TimedWait(lock, timeout);
                } else {
                    enqueue_cond.Wait(lock);
                }
                --waiting;
            }
            shared_ptr<Runnable> r = taskqueue.front();
            taskqueue.pop_front();
            al.Unlock();
            if (r) r->Run();
            al.Lock();
            --numtasks;
            dequeue_cond.Signal();
        }
    }

}
