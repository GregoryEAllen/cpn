
#pragma once

namespace Sync {
    class Runnable {
    public:
        virtual ~Runnable();
        virtual void Run() = 0;
    };
}

