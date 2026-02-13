
#include "ThreadPool.h"
#include "ExClass.h"
#include <cassert>
#include <vector>
#include <unistd.h>

using std::auto_ptr;
using namespace Sync;
using std::vector;

void ExFunc(ExClass *ex) {
    ex->func1();
}

int SomeFunc(int i) {
    ExClass ex1;
    sleep(1);
    printf("SomeFunc(%d)\n", i);
    return i + 1;
}

int main(int argc, char **argv) {
    ExClass ex1;

    shared_ptr<Future<void> > voidfun1;
    shared_ptr<Future<void> > voidfun2;
    shared_ptr<Future<int> > intfun;

    ThreadPool pool;

    voidfun1 = pool.Execute(&ex1, &ExClass::func1);
    assert(voidfun1);
    voidfun2 = pool.Execute(&ExFunc, &ex1);
    assert(voidfun2);
    intfun = pool.Execute(&SomeFunc, 1);
    assert(intfun);

    voidfun1->Get();
    voidfun2->Get();
    assert(2 == intfun->Get());

    typedef vector<shared_ptr<Future<int> > > Futures;
    Futures futures;
    for (int i = 0; i < 20; ++i) {
        futures.push_back(pool.Execute(&SomeFunc, i));
    }
    pool.Wait();
    for (Futures::iterator itr = futures.begin(), end = futures.end();
            itr != end; ++itr) {
        printf("Joined %d\n", (*itr)->Get());
    }
    return 0;
}

