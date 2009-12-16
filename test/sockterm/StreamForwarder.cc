
#include "StreamForwarder.h"
#include "Assert.h"

StreamForwarder::StreamForwarder()
    : forward(0), buff(BUFF_SIZE)
{
    printf("%s\n",__PRETTY_FUNCTION__);
}

void StreamForwarder::OnRead() {
    printf("%s\n",__PRETTY_FUNCTION__);
    if (!forward) { return; }
    unsigned numtoread = 0;
    char* in = forward->AllocatePut(buff.MaxSize(), numtoread);
    unsigned numread = Read(in, numtoread);
    if (numread == 0) {
        if (Eof()) {
            printf("Read EOF\n");
            Readable(false);
        } else {
            printf("Read 0 bytes from input!\n");
        }
    } else {
        forward->ReleasePut(numread);
    }
}

void StreamForwarder::OnWrite() {
    printf("%s\n",__PRETTY_FUNCTION__);
    unsigned numtowrite = 0;
    char* out = buff.AllocateGet(buff.Size(), numtowrite);
    if (numtowrite > 0) {
        unsigned numwritten = Write(out, numtowrite);
        buff.ReleaseGet(numwritten);
    } else {
        Writeable(false);
    }
}

void StreamForwarder::OnError() {
    printf("%s\n",__PRETTY_FUNCTION__);
}

void StreamForwarder::OnHup() {
    printf("%s\n",__PRETTY_FUNCTION__);
}
void StreamForwarder::OnInval() {
    printf("%s\n",__PRETTY_FUNCTION__);
}
