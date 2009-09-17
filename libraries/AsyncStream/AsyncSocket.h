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
 * \brief Classes and definitions for the async sockets
 * \author John Bridgman
 */

#ifndef ASTREAM_ASYNCSOCKET_H
#define ASTREAM_ASYNCSOCKET_H
#pragma once

#include "AsyncStream.h"
#include <tr1/memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <vector>
#include <string>

struct addrinfo;

namespace Async {

    class StreamSocket;
    class ListenSocket;
    class SocketAddress;
    typedef std::vector<SocketAddress> SockAddrList;
    typedef std::tr1::shared_ptr<StreamSocket> SockPtr;
    typedef std::tr1::shared_ptr<ListenSocket> ListenSockPtr;

    class SocketAddress {
        friend class StreamSocket;
        friend class ListenSocket;
    public:

        enum Type_t {
            IPV4,
            IPV6,
            LOCAL
        };

        /**
         * \brief Return a list of valid socket addresses for the given service name
         * All CreateIP functions create IP addresses.
         * \return a list of SocketAddresses
         */
        static SockAddrList CreateIPFromServ(const char* servname);
        static SockAddrList CreateIPFromHost(const char* hostname);
        /// \brief Return a list of valid socket address for the given service number
        /// or port number
        static SockAddrList CreateIP(unsigned serv);
        /**
         * \param hostname The hostname to lookup
         * \param servname the service name to lookup, may be a number string for the port
         * \return a list of valid addresses to connect to the given host/port
         */
        static SockAddrList CreateIP(const char* hostname, const char* servname);
        /**
         * \param hostname the hostname to lookup
         * \param serv the port number
         * \return a list of valid address to connect to the given host/port
         */
        static SockAddrList CreateIP(const char* hostname, unsigned serv);
        /**
         * Create a SocketAddress from the underlying sockaddr and length
         * \param addr pointer tot he address structure
         * \param length the length of the address structure
         * \return an SocketAddress
         */
        static SocketAddress Create(sockaddr *addr, socklen_t len);

        SocketAddress() : length(0) {}
        ~SocketAddress() {}

        std::string GetHostName() const;
        std::string GetServName() const;
        unsigned GetServ() const;

        Type_t GetType() const;

    protected:
        SocketAddress(addrinfo *info);
        SocketAddress(addrinfo *info, unsigned portnum);
        SocketAddress(sockaddr *addr, socklen_t len);

        sockaddr *GetAddr() { return &address.addr; }
        socklen_t &GetLen() { return length; }
        sa_family_t &Family() { return address.storage.ss_family; }
        sa_family_t Family() const { return address.storage.ss_family; }

    private:
        static SockAddrList Lookup(const char* const hostname, const char* const port,
                int family, unsigned portnum);

        socklen_t length;

        union address_ {
            sockaddr addr;
            sockaddr_in in;
            sockaddr_in6 in6;
            sockaddr_un un;
            sockaddr_storage storage;
        } address;
    };


    class StreamSocket : public Descriptor {
        friend class ListenSocket;
    public:

        static SockPtr Create(const SocketAddress &address);
        static void CreatePair(SockPtr &sock1, SockPtr &sock2);

        virtual ~StreamSocket() throw();

        const SocketAddress &GetLocalAddress() const { return localaddress; }
        const SocketAddress &GetRemoteAddress() const { return remoteaddress; }

    private:
        StreamSocket(const SocketAddress &address);
        StreamSocket(int fid, const SocketAddress &address);
        StreamSocket(int fid);

        SocketAddress remoteaddress;
        SocketAddress localaddress;
    };

    /** When a ListenSocket becomes readable one may Accept a connection. Note
     * that a ListenSocket cannot be Read from or Written to.
     */
    class ListenSocket : public Descriptor {
    public:

        static ListenSockPtr Create(const SocketAddress &address);

        virtual ~ListenSocket() throw();

        /**
         * Can fail return should be checked.
         */
        SockPtr Accept();

        const SocketAddress &GetAddress() const { return address; }

    private:
        ListenSocket(const SocketAddress &addr);
        SocketAddress address;
    };


}

#endif

