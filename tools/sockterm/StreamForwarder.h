
#pragma once

#include "CircularQueue.h"
#include "SocketHandle.h"
#include <algorithm>

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
        actual = requested;
        actual = std::min(actual, buff.MaxThreshold());
        actual = std::min(actual, buff.Freespace());
        return (char*)buff.GetRawEnqueuePtr(actual);
    }

    void ReleasePut(unsigned num) {
        buff.Enqueue(num);
    }

    bool Good() { return handle.Good(); }

    SocketHandle &GetHandle() { return handle; }
private:

    const char *AllocateGet(unsigned requested, unsigned &actual) {
        actual = requested;
        actual = std::min(actual, buff.MaxThreshold());
        actual = std::min(actual, buff.Count());
        return (const char*)buff.GetRawDequeuePtr(actual);
    }

    void ReleaseGet(unsigned num) {
        buff.Dequeue(num);
    }

    SocketHandle handle;
    StreamForwarder *forward;
    CircularQueue buff;
};

