
#pragma once

#include "AutoCircleBuffer.h"
#include "SocketHandle.h"

const int BUFF_SIZE = 1024;

class StreamForwarder {
public:
    StreamForwarder();

    void Read();
    void Write();

    void SetForward(StreamForwarder *f) {
        forward = f;
    }

    char *AllocatePut(unsigned requested, unsigned &actual) {
        return buff.AllocatePut(requested, actual);
    }

    void ReleasePut(unsigned num) {
        buff.ReleasePut(num);
    }

    unsigned Size() const {
        return buff.Size();
    }

    bool Good() { return handle.Good(); }

    SocketHandle &GetHandle() { return handle; }
private:
    SocketHandle handle;
    StreamForwarder *forward;
    AutoCircleBuffer buff;
};

