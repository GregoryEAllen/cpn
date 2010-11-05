
#include "StreamForwarder.h"
#include "Assert.h"
#include <stdio.h>

StreamForwarder::StreamForwarder()
    : forward(0), buff(BUFF_SIZE, BUFF_SIZE, 1)
{
    printf("%s\n",__PRETTY_FUNCTION__);
    handle.Writeable(true);
}

void StreamForwarder::Read() {
    if (!forward) { return; }
    while (handle.Readable()) {
        printf("%s\n",__PRETTY_FUNCTION__);
        unsigned numtoread = 0;
        char* in = forward->AllocatePut(buff.MaxThreshold(), numtoread);
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
        const char* out = AllocateGet(buff.MaxThreshold(), numtowrite);
        if (numtowrite > 0) {
            unsigned numwritten = handle.Write(out, numtowrite);
            ReleaseGet(numwritten);
        } else { break; }
    }
}

