/** \file
 */

#include "SocketAddress.h"
#include <netdb.h>
#include <err.h>
#include <cassert>
#include <cstring>

using namespace Socket;

int SocketAddress::Lookup(const char* const hostname, const char* const port,
        SockFamily family, std::vector<SocketAddress> &results) {
    addrinfo hints = {0};
    // Get only supported types.
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_family = (int)family;
    hints.ai_socktype = SOCK_STREAM;
    if (0 == hostname) {
        hints.ai_flags |= AI_PASSIVE;
    }
    addrinfo *res = 0;
    int lookupstatus = 0;
    do {
    lookupstatus = getaddrinfo(hostname, port, &hints, &res);
    switch (lookupstatus) {
        case EAI_AGAIN:
            // try again
        case EAI_MEMORY:
            // our of memory, try again?
        case 0:
            // Success.
            break;
        case EAI_ADDRFAMILY:
        case EAI_BADFLAGS:
        case EAI_FAIL:
        case EAI_FAMILY:
        case EAI_NODATA:
        case EAI_NONAME:
        case EAI_SERVICE:
        case EAI_SOCKTYPE:
        case EAI_SYSTEM:
        default:
            warn("SocketAddress::Lookup: %s", gai_strerror(lookupstatus));
            return lookupstatus;
    }
    } while (lookupstatus != 0);

    for (addrinfo *rp = res; rp != 0; rp = rp->ai_next) {
        results.push_back(SocketAddress(rp));
    }
    freeaddrinfo(res);
    return 0;
}

SocketAddress::SocketAddress() 
: length(sizeof(sockaddr_in))
{
    Family(0) = PF_INET;
    address.in.sin_addr.s_addr = INADDR_ANY;
    name = "0.0.0.0";
}

SocketAddress::SocketAddress(unsigned port) 
: length(sizeof(sockaddr_in))
{
    Family(0) = PF_INET;
    address.in.sin_addr.s_addr = INADDR_ANY;
    address.in.sin_port = port;
    name = "0.0.0.0";
}

SocketAddress::SocketAddress(addrinfo *ai) 
{
    length = ai->ai_addrlen;
    memcpy(&address, ai->ai_addr, length);
}

SocketAddress::SocketAddress(const char* const name_, SockFamily family) {
    switch (family) {
#if 0
    case LOCAL: {
            Family() = family;
            length = sizeof(sockaddr_un);
            name = name_;
            int size = sizeof(address.un.sun_path);
            strncpy(address.un.sun_path, name_, size);
            address.un.sun_path[size - 1] = '\0';
        }
        break;
#endif
    case INET:
    case INET6:
    case FAM_UNSPEC: {
            std::vector<SocketAddress> results;
            Lookup(name_, 0, family, results);
            *this = results.front();
        }
        break;
    default:
        assert(false);
    }
}

SocketAddress::~SocketAddress() {}

unsigned SocketAddress::Port(void) const {
    switch (Family()) {
    case INET:
        return address.in.sin_port;
    case INET6:
        return address.in6.sin6_port;
    default:
        assert(false);
        break;
    }
    return -1;
}

void SocketAddress::Port(unsigned p) {
    switch (Family()) {
    case INET:
        address.in.sin_port = p;
        break;
    case INET6:
        address.in6.sin6_port = p;
        break;
    default:
        assert(false);
        break;
    }
}

