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
#ifndef SOCKETADDRESS_H
#define SOCKETADDRESS_H
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <vector>
#include <string>

class SocketAddress;

typedef std::vector<SocketAddress> SockAddrList;

/**
 * \brief An abstraction of a socket address with convenience methods.
 */
class SocketAddress {
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
    static SockAddrList CreateIPFromServ(const std::string &servname);
    static SockAddrList CreateIPFromHost(const char* hostname);
    static SockAddrList CreateIPFromHost(const std::string &hostname);
    /// \brief Return a list of valid socket address for the given service number
    /// or port number
    static SockAddrList CreateIP(unsigned serv);
    /**
     * \param hostname The hostname to lookup
     * \param servname the service name to lookup, may be a number string for the port
     * \return a list of valid addresses to connect to the given host/port
     */
    static SockAddrList CreateIP(const char* hostname, const char* servname);
    static SockAddrList CreateIP(const std::string &hostname, const std::string &servname);
    /**
     * \param hostname the hostname to lookup
     * \param serv the port number
     * \return a list of valid address to connect to the given host/port
     */
    static SockAddrList CreateIP(const char* hostname, unsigned serv);
    static SockAddrList CreateIP(const std::string &hostname, unsigned serv);

    SocketAddress();
    SocketAddress(addrinfo *info);
    SocketAddress(addrinfo *info, unsigned portnum);
    /**
     * Create a SocketAddress from the underlying sockaddr and length
     * \param addr pointer tot he address structure
     * \param len the length of the address structure
     */
    SocketAddress(sockaddr *addr, socklen_t len);
    ~SocketAddress() {}

    /**
     * \param numerichost true to force no resolution of host names and just return the number
     * \return a string representation of the host name
     */
    std::string GetHostName(bool numerichost = false) const;
    /**
     * \return the service (port) name or number as a string.
     */
    std::string GetServName() const;
    /**
     * \return the service (port) number
     */
    unsigned GetServ() const;

    Type_t GetType() const;


    sockaddr *GetAddr() { return &address.addr; }
    socklen_t &GetLen() { return length; }
    sa_family_t &Family() { return address.storage.ss_family; }
    sa_family_t Family() const { return address.storage.ss_family; }

    /**
     * \brief Fill this SocketAddress with data from this side of the
     * connection represented by fd.
     * \param fd the socket to use
     */
    void SetFromSockName(int fd);
    /**
     * \brief Fill this SocketAddress with data from the other side of the
     * connection represented by fd.
     * \param fd the socket to use
     */
    void SetFromPeerName(int fd);

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

#endif
