
#pragma once
#include "Runnable.h"
#include "Future.h"

namespace Sync {

    template<typename T>
    class RunnableFuture : public Runnable, public Future<T> {};

}

