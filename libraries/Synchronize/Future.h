
#pragma once
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "AutoLock.h"

template<typename T>
class Future {
public:
    Future() : done(false), canceled(false), ret() {}
    virtual ~Future() {}

    bool Done() {
        AutoLock<PthreadMutex> al(future_lock);
        return done || canceled;
    }

    void Cancel() {
        AutoLock<PthreadMutex> al(future_lock);
        canceled = true;
        future_cond.Broadcast();
    }

    void Wait() {
        AutoLock<PthreadMutex> al(future_lock);
        InternalWait();
    }

    bool IsCanceled() {
        AutoLock<PthreadMutex> al(future_lock);
        return canceled;
    }

    virtual void Set(const T &r) {
        AutoLock<PthreadMutex> al(future_lock);
        InternalSet(r);
    }

    virtual T Get() {
        AutoLock<PthreadMutex> al(future_lock);
        InternalWait();
        return ret;
    }

protected:
    void InternalWait() {
        while (!canceled && !done) { future_cond.Wait(future_lock); }
    }

    void InternalSet(const T &r) {
        ret = r;
        done = true;
        future_cond.Broadcast();
    }

    PthreadMutex future_lock;
    PthreadCondition future_cond;
    bool done;
    bool canceled;
    T ret;
};

template<>
class Future<void> {
public:
    Future() : done(false), canceled(false) {}
    virtual ~Future() {}

    bool Done() {
        AutoLock<PthreadMutex> al(future_lock);
        return done || canceled;
    }

    void Cancel() {
        AutoLock<PthreadMutex> al(future_lock);
        canceled = true;
        future_cond.Broadcast();
    }

    void Wait() {
        AutoLock<PthreadMutex> al(future_lock);
        InternalWait();
    }

    bool IsCanceled() {
        AutoLock<PthreadMutex> al(future_lock);
        return canceled;
    }

    virtual void Set() {
        AutoLock<PthreadMutex> al(future_lock);
        InternalSet();
    }

    virtual void Get() {
        AutoLock<PthreadMutex> al(future_lock);
        InternalWait();
    }

protected:
    void InternalWait() {
        while (!canceled && !done) { future_cond.Wait(future_lock); }
    }

    void InternalSet() {
        done = true;
        future_cond.Broadcast();
    }

    PthreadMutex future_lock;
    PthreadCondition future_cond;
    bool done;
    bool canceled;
};


