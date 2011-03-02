//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */

#include "SocketAddress.h"
#include "ErrnoException.h"
#include "ThrowingAssert.h"
#include <string.h>
#include <netdb.h>
#include <sstream>
#include <err.h>
#include <errno.h>

SocketAddress::SocketAddress()
    : length(0)
{
    memset(&address, 0, sizeof(address));
}

SockAddrList SocketAddress::Lookup(const char* const hostname, const char* const port,
        int family, unsigned portnum) {
    addrinfo hints = {0};
    // Get only supported types.
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_family = family;
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
                throw ErrnoException(gai_strerror(lookupstatus), lookupstatus);
        }
    } while (lookupstatus != 0);

    SockAddrList results;
    for (addrinfo *rp = res; rp != 0; rp = rp->ai_next) {
        if (port) {
            results.push_back(SocketAddress(rp));
        } else {
            results.push_back(SocketAddress(rp, portnum));
        }
    }
    freeaddrinfo(res);
    return results;
}

SockAddrList SocketAddress::CreateIPFromServ(const char* servname) {
    return Lookup(0, servname, AF_UNSPEC, 0);
}

SockAddrList SocketAddress::CreateIPFromServ(const std::string &servname) {
    return Lookup(0, servname.c_str(), AF_UNSPEC, 0);
}

SockAddrList SocketAddress::CreateIPFromHost(const char* hostname) {
    return Lookup(hostname, 0, AF_UNSPEC, 0);
}

SockAddrList SocketAddress::CreateIPFromHost(const std::string &hostname) {
    return Lookup(hostname.c_str(), 0, AF_UNSPEC, 0);
}

SockAddrList SocketAddress::CreateIP(unsigned serv) {
    return Lookup(0, 0, AF_UNSPEC, serv);
}

SockAddrList SocketAddress::CreateIP(const char* hostname, const char* servname) {
    return Lookup(hostname, servname, AF_UNSPEC, 0);
}

SockAddrList SocketAddress::CreateIP(const std::string &hostname, const std::string &servname) {
    return Lookup(hostname.c_str(), servname.c_str(), AF_UNSPEC, 0);
}

SockAddrList SocketAddress::CreateIP(const char *hostname, unsigned serv) {
    return Lookup(hostname, 0, AF_UNSPEC, serv);
}

SockAddrList SocketAddress::CreateIP(const std::string &hostname, unsigned serv) {
    return Lookup(hostname.c_str(), 0, AF_UNSPEC, serv);
}

SocketAddress::SocketAddress(addrinfo *info) {
    length = info->ai_addrlen;
    memcpy(&address, info->ai_addr, length);
}

SocketAddress::SocketAddress(addrinfo *info, unsigned portnum) {
    length = info->ai_addrlen;
    memcpy(&address, info->ai_addr, length);
    uint16_t port = (uint16_t)portnum;
    port = htons(port);
    switch (Family()) {
    case AF_INET:
        address.in.sin_port = port;
        break;
    case AF_INET6:
        address.in6.sin6_port = port;
        break;
    default:
        break;
    }
}

SocketAddress::SocketAddress(sockaddr *addr, socklen_t len) {
    memcpy(&address, addr, len);
    length = len;
}

std::string SocketAddress::GetHostName(bool numerichost) const {
    std::vector<char> hostname(NI_MAXHOST, '\0');
    bool loop = true;
    std::ostringstream oss;
    while (loop) {
        int flags = NI_NUMERICSERV;
        if (numerichost) { flags |= NI_NUMERICHOST; }
        int lookupstatus = getnameinfo(&address.addr, length,
                    &hostname[0], hostname.size(),
                    0, 0,
                    flags);
        switch (lookupstatus) {
        case 0:
            oss << &hostname[0];
            loop = false;
            break;
        case EAI_AGAIN:
            break;
#if !defined(OS_DARWIN)
        case EAI_OVERFLOW:
            hostname.resize(hostname.size()*2, '\0');
            break;
#endif
        case EAI_BADFLAGS:
        case EAI_FAIL:
        case EAI_FAMILY:
        case EAI_MEMORY:
        case EAI_NONAME:
        case EAI_SYSTEM:
        default:
            throw ErrnoException(gai_strerror(lookupstatus), lookupstatus);
        }
    }
    return oss.str();
}

std::string SocketAddress::GetServName() const {
    std::vector<char> servname(NI_MAXSERV, '\0');
    bool loop = true;
    std::ostringstream oss;
    while (loop) {
        int lookupstatus = getnameinfo(&address.addr, length,
                    0, 0,
                    &servname[0], servname.size(),
                    NI_NUMERICSERV);
        switch (lookupstatus) {
        case 0:
            oss << &servname[0];
            loop = false;
            break;
        case EAI_AGAIN:
            break;
#if !defined(OS_DARWIN)
        case EAI_OVERFLOW:
            servname.resize(servname.size()*2, '\0');
            break;
#endif
        case EAI_BADFLAGS:
        case EAI_FAIL:
        case EAI_FAMILY:
        case EAI_MEMORY:
        case EAI_NONAME:
        case EAI_SYSTEM:
        default:
            throw ErrnoException(gai_strerror(lookupstatus), lookupstatus);
        }
    }
    return oss.str();
}

unsigned SocketAddress::GetServ() const {
    uint16_t retval = 0;
    switch (Family()) {
    case AF_INET:
        retval = address.in.sin_port;
        break;
    case AF_INET6:
        retval = address.in6.sin6_port;
        break;
    default:
        break;
    }
    return (unsigned)ntohs(retval);
}

SocketAddress::Type_t SocketAddress::GetType() const {
    Type_t type;
    switch (Family()) {
    case AF_INET:
        type = IPV4;
        break;
    case AF_INET6:
        type = IPV6;
        break;
    case AF_UNIX:
        type = LOCAL;
        break;
    default:
        ASSERT(false, "Unknown address type.");
    }
    return type;
}

void SocketAddress::SetFromSockName(int fd) {
    GetLen() = sizeof(sockaddr_storage);
    if (getsockname(fd, GetAddr(), &GetLen()) != 0) {
        throw ErrnoException();
    }
}

void SocketAddress::SetFromPeerName(int fd) {
    GetLen() = sizeof(sockaddr_storage);
    if (getpeername(fd, GetAddr(), &GetLen()) != 0) {
        throw ErrnoException();
    }
}


