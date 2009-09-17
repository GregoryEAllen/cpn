
#include "StreamForwarder.h"

using Async::Stream;
using Async::Descriptor;
using Async::DescriptorPtr;

StreamForwarder::StreamForwarder(Stream read, Stream write)
    : reader(read), writer(write), buff(BUFF_SIZE)
{
    //printf("%s\n",__PRETTY_FUNCTION__);
    DescriptorPtr fd = read.Fd();
    fd->ConnectReadable(sigc::mem_fun(this, &StreamForwarder::CheckRead));
    fd->ConnectOnRead(sigc::mem_fun(this, &StreamForwarder::Read));
    fd->ConnectOnError(sigc::mem_fun(this, &StreamForwarder::Error));
    fd = write.Fd();
    fd->ConnectWriteable(sigc::mem_fun(this, &StreamForwarder::CheckWrite));
    fd->ConnectOnWrite(sigc::mem_fun(this, &StreamForwarder::Write));
    fd->ConnectOnError(sigc::mem_fun(this, &StreamForwarder::Error));
}

void StreamForwarder::Read() {
    //printf("%s\n",__PRETTY_FUNCTION__);
    unsigned numtoread = 0;
    char* in = buff.AllocatePut(buff.MaxSize(), numtoread);
    unsigned numread = reader.Read(in, numtoread);
    if (numread == 0) {
        if (reader.Fd()->Eof()) {
            printf("Read EOF\n");
        } else {
            printf("Read 0 bytes from input!\n");
        }
    } else {
        buff.ReleasePut(numread);
    }
}
void StreamForwarder::Write() {
    //printf("%s\n",__PRETTY_FUNCTION__);
    unsigned numtowrite = 0;
    char* out = buff.AllocateGet(buff.Size(), numtowrite);
    unsigned numwritten = writer.Write(out, numtowrite);
    if (numwritten < 0) {
        perror("Error on write");
    } else {
        buff.ReleaseGet(numwritten);
    }
}

bool StreamForwarder::Good() {
    return *(reader.Fd()) && *(writer.Fd());
}
