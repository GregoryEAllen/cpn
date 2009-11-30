
#pragma once

template<typename lockable>
struct AutoUnlock {
    AutoUnlock(lockable &l) : lock(l) { lock.Unlock(); }
    ~AutoUnlock() { lock.Lock(); }
    lockable &lock;
};

