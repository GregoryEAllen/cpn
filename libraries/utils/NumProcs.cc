
#include "NumProcs.h"
#include "ErrnoException.h"
#include <sched.h>

#ifdef OS_LINUX

void SetNumProcs(int n) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < n; ++i) {
        CPU_SET(i, &mask);
    }
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        throw ErrnoException();
    }
}

int GetNumProcs() {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    if (sched_getaffinity(0, sizeof(mask), &mask) == -1) {
        throw ErrnoException();
    }
    int num = 0;
    for (int i = 0; i < CPU_SETSIZE; ++i) {
        if (CPU_ISSET(i, &mask)) {
            ++num;
        }
    }
    return num;
}

#else

#include "SysConf.h"

int GetNumProcs() {
    return NumProcessorsOnline();
}

#endif
