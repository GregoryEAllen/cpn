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

#include "AsyncSocket.h"
#include "Assert.h"
#include <netdb.h>
#include <sstream>
#include <err.h>
#include <errno.h>
#include <exception>

namespace Async {

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
                    throw StreamException(gai_strerror(lookupstatus), lookupstatus);
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

    SockAddrList SocketAddress::CreateIPFromHost(const char* hostname) {
        return Lookup(hostname, 0, AF_UNSPEC, 0);
    }

    SockAddrList SocketAddress::CreateIP(unsigned serv) {
        return Lookup(0, 0, AF_UNSPEC, serv);
    }

    SockAddrList SocketAddress::CreateIP(const char* hostname, const char* servname) {
        return Lookup(hostname, servname, AF_UNSPEC, 0);
    }

    SockAddrList SocketAddress::CreateIP(const char *hostname, unsigned serv) {
        return Lookup(hostname, 0, AF_UNSPEC, serv);
    }

    SocketAddress SocketAddress::Create(sockaddr *addr, socklen_t len) {
        return SocketAddress(addr, len);
    }

    SocketAddress::SocketAddress(addrinfo *info) {
        length = info->ai_addrlen;
        memcpy(&address, info->ai_addr, length);
    }

    SocketAddress::SocketAddress(addrinfo *info, unsigned portnum) {
        length = info->ai_addrlen;
        memcpy(&address, info->ai_addr, length);
        switch (Family()) {
        case AF_INET:
            address.in.sin_port = portnum;
            break;
        case AF_INET6:
            address.in6.sin6_port = portnum;
            break;
        default:
            warn("%s unknown address type.", __PRETTY_FUNCTION__);
            break;
        }
    }

    SocketAddress::SocketAddress(sockaddr *addr, socklen_t len) {
        memcpy(&address, addr, len);
        length = len;
    }

    std::string SocketAddress::GetHostName() const {
        std::vector<char> hostname(NI_MAXHOST, '\0');
        bool loop = true;
        std::ostringstream oss;
        while (loop) {
            int lookupstatus = getnameinfo(&address.addr, length,
                        &hostname[0], hostname.size(),
                        0, 0,
                        NI_NUMERICSERV);
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
                warn("SocketAddress::Lookup: %s", gai_strerror(lookupstatus));
                loop = false;
                break;
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
                warn("SocketAddress::Lookup: %s", gai_strerror(lookupstatus));
                loop = false;
                break;
            }
        }
        return oss.str();
    }

    unsigned SocketAddress::GetServ() const {
        unsigned retval = 0;
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
        return retval;
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

    SockPtr StreamSocket::Create(const SocketAddress &address) {
        return SockPtr(new StreamSocket(address));
    }
    SockPtr StreamSocket::Create(const SockAddrList &addresses) {
        return SockPtr(new StreamSocket(addresses));
    }

    void StreamSocket::CreatePair(SockPtr &sock1, SockPtr &sock2) {
        int pair[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) < 0) {
            throw StreamException(errno);
        }
        sock1 = SockPtr(new StreamSocket(pair[0]));
        sock2 = SockPtr(new StreamSocket(pair[1]));
    }

    StreamSocket::StreamSocket(const SocketAddress &addr) {
        remoteaddress = addr;
        int error = 0;
        if (!Connect(error)) {
            throw StreamException(error);
        }
        SetNonBlocking(true);
    }

    StreamSocket::StreamSocket(const SockAddrList &addrs) {
        int error = 0;
        bool success = false;
        for (SockAddrList::const_iterator itr = addrs.begin();
                itr != addrs.end(); ++itr) {
            remoteaddress = *itr;
            success = Connect(error);
            if (success) break;
        }
        if (!success) throw StreamException(error);
        SetNonBlocking(true);
    }

    StreamSocket::StreamSocket(int fid, const SocketAddress &address)
    : Descriptor(fid), remoteaddress(address) {
        localaddress.GetLen() = sizeof(sockaddr_storage);
        getsockname(fd, localaddress.GetAddr(), &localaddress.GetLen());
    }

    StreamSocket::StreamSocket(int fid)
    : Descriptor(fid) {
        LookupRemoteAddress();
        LookupLocalAddress();
    }

    StreamSocket::~StreamSocket() throw() {
    }

    bool StreamSocket::Connect(int &error) {
        fd = socket(remoteaddress.Family(), SOCK_STREAM, 0);
        if (fd < 0) {
            error = errno;
            return false;
        }
        bool loop = true;
        while (loop) {
            if (connect(fd, remoteaddress.GetAddr(), remoteaddress.GetLen()) < 0) {
                error = errno;
                switch (error) {
                case EINTR:
                case EAGAIN: // not enough resources
                    break;
                case EINPROGRESS: // non blocking and connection not completed yet
                    // may want to change things to let non blocking connect.
                case EALREADY: // previous attempt has not completed
                case EACCES:
                case EPERM:
                case EADDRINUSE:
                case EAFNOSUPPORT:
                case EBADF:
                case ECONNREFUSED:
                case EFAULT:
                case EISCONN:
                case ENETUNREACH:
                case ENOTSOCK:
                case ETIMEDOUT:
                default:
                    close(fd);
                    fd = -1;
                    return false;
                }
            } else {
               loop = false;
               LookupRemoteAddress();
            }
        }
        return true;
    }

    void StreamSocket::LookupRemoteAddress() {
        remoteaddress.GetLen() = sizeof(sockaddr_storage);
        getpeername(fd, remoteaddress.GetAddr(), &remoteaddress.GetLen());
    }

    void StreamSocket::LookupLocalAddress() {
        localaddress.GetLen() = sizeof(sockaddr_storage);
        getsockname(fd, localaddress.GetAddr(), &localaddress.GetLen());
    }

    ListenSockPtr ListenSocket::Create(const SocketAddress &address) {
        return ListenSockPtr(new ListenSocket(address));
    }

    ListenSockPtr ListenSocket::Create(const SockAddrList &addresses) {
        return ListenSockPtr(new ListenSocket(addresses));
    }

    ListenSocket::ListenSocket(const SocketAddress &addr)
    : address(addr) {
        int error = 0;
        if (!Listen(error)) {
            throw StreamException(error);
        }
    }

    ListenSocket::ListenSocket(const SockAddrList &addrs) {
        int error = 0;
        bool success = false;
        for (SockAddrList::const_iterator itr = addrs.begin();
                itr != addrs.end(); ++itr) {
            address = *itr;
            success = Listen(error);
            if (success) break;
        }
        if (!success) throw StreamException(error);
    }

    ListenSocket::~ListenSocket() throw() {
    }

    bool ListenSocket::Listen(int &error) {
        fd = socket(address.Family(), SOCK_STREAM, 0);
        if (fd < 0) {
            error = errno;
            return false;
        }
        if (bind(fd, address.GetAddr(), address.GetLen()) < 0) {
            error = errno;
            return false;
        }
        if (listen(fd, 10) < 0) {
            error = errno;
            return false;
        }
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        address.GetLen() = sizeof(sockaddr_storage);
        getsockname(fd, address.GetAddr(), &address.GetLen());
        return true;
    }

    SockPtr ListenSocket::Accept() {
        bool loop = true;
        int nfd = -1;
        SocketAddress raddress;
        while (loop) {
            raddress.GetLen() = sizeof(sockaddr_storage);
            nfd = accept(fd, raddress.GetAddr(), &raddress.GetLen());
            if (nfd < 0) {
                int error = errno;
                switch (error) {
                case EAGAIN:
                //case EWOULDBLOCK: // Duplicate
                case ECONNABORTED:
                case EINTR:
                case ENETDOWN:
                case EPROTO:
                case ENOPROTOOPT:
                case EHOSTDOWN:
                //case ENONET:
                case EHOSTUNREACH:
                case ENETUNREACH:
                case ENOBUFS:
                case ENOMEM:
                case EPERM:
                case EPIPE:
                case ETIMEDOUT:
                    nfd = -1;
                    loop = false;
                    break;
                case EOPNOTSUPP:
                case EBADF:
                case EINVAL:
                case ENFILE:
                case EMFILE:
                case ENOTSOCK:
                case EFAULT:
                default:
                    loop = false;
                    nfd = 0;
                    throw StreamException(error);
                    break;
                }
            } else {
                return SockPtr(new StreamSocket(nfd, raddress));
            }
        }
        return SockPtr();
    }
}

