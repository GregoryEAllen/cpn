
#pragma once

#include "AutoCircleBuffer.h"
#include "SockHandler.h"

const int BUFF_SIZE = 1024;

class StreamForwarder : public SockHandler {
public:
    StreamForwarder();

    void OnRead();
    void OnWrite();
    void OnError();
    void OnHup();
    void OnInval();

    void SetForward(StreamForwarder *f) {
        forward = f;
        if (forward) { Readable(true); }
    }

    char *AllocatePut(unsigned requested, unsigned &actual) {
        return buff.AllocatePut(requested, actual);
    }

    void ReleasePut(unsigned num) {
        buff.ReleasePut(num);
        if (Size() > 0) { Writeable(true); }
    }

    unsigned Size() const {
        return buff.Size();
    }
private:
    StreamForwarder *forward;
    AutoCircleBuffer buff;
};

