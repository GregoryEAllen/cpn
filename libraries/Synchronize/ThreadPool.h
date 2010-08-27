//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */
#ifndef SYNC_THREADPOOL_H
#define SYNC_THREADPOOL_H
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
#endif
