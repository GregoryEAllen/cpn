
#pragma once

#include "AutoCircleBuffer.h"
#include "AsyncStream.h"


const int BUFF_SIZE = 1024;

class StreamForwarder : public sigc::trackable {
public:
    StreamForwarder(Async::Stream read, Async::Stream write);

    bool CheckRead() {
        //printf("%s\n",__PRETTY_FUNCTION__);
        return buff.Size() < buff.MaxSize();
    }
    bool CheckWrite() {
        //printf("%s\n",__PRETTY_FUNCTION__);
        return buff.Size() > 0;
    }

    void Read();
    void Write();
    bool Good();
    void Error(int err) {
        printf("An error on a stream %d\n", err);
    }
private:
    Async::Stream reader;
    Async::Stream writer;
    AutoCircleBuffer buff;
};

