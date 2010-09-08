
#include "StreamForwarder.h"
#include "Assert.h"
#include <stdio.h>

StreamForwarder::StreamForwarder()
    : forward(0), buff(BUFF_SIZE)
{
    printf("%s\n",__PRETTY_FUNCTION__);
    handle.Writeable(true);
}

void StreamForwarder::Read() {
    if (!forward) { return; }
    while (handle.Readable()) {
        printf("%s\n",__PRETTY_FUNCTION__);
        unsigned numtoread = 0;
        char* in = forward->AllocatePut(buff.MaxSize(), numtoread);
        if (numtoread == 0) { break; }
        unsigned numread = handle.Read(in, numtoread);
        if (numread == 0) {
            if (handle.Eof()) {
                printf("Read EOF\n");
            } else {
                printf("Read 0 bytes from input!\n");
            }
        } else {
            forward->ReleasePut(numread);
        }
    }
}

void StreamForwarder::Write() {
    while (handle.Writeable()) {
        printf("%s\n",__PRETTY_FUNCTION__);
        unsigned numtowrite = 0;
        char* out = buff.AllocateGet(buff.Size(), numtowrite);
        if (numtowrite > 0) {
            unsigned numwritten = handle.Write(out, numtowrite);
            buff.ReleaseGet(numwritten);
        } else { break; }
    }
}

