
#include "PacketHeader.h"

#include <unistd.h>
#include <stdio.h>


int main(int argc, char **argv) {
    printf("PacketHeaderBase size = %lu\n", sizeof(CPN::PacketHeaderBase));
    printf("EnqueuePacketHeader size = %lu\n", sizeof(CPN::EnqueuePacketHeader));
    printf("DequeuePacketHeader size = %lu\n", sizeof(CPN::DequeuePacketHeader));
    printf("ReadBlockPacketHeader size = %lu\n", sizeof(CPN::ReadBlockPacketHeader));
    printf("WriteBlockPacketheader size = %lu\n", sizeof(CPN::WriteBlockPacketHeader));
    printf("CreateQueuePacketHeader size = %lu\n", sizeof(CPN::CreateQueuePacketHeader));
    printf("CreateNodePacketHeader size = %lu\n", sizeof(CPN::CreateNodePacketHeader));
    printf("SocketEndpointHeader size = %lu\n", sizeof(CPN::SocketEndpointHeader));
}

