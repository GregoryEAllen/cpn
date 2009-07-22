/** \file
 */

#ifndef SOCKET_SOCKETADDRESS_H
#define SOCKET_SOCKETADDRESS_H
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <sys/un.h>
#include <string>
#include <vector>

struct addrinfo;

namespace Socket {

    /**
     * These are the socket family types we actually support.
     */
    enum SockFamily {
        FAM_UNSPEC = AF_UNSPEC,
        //LOCAL = PF_UNIX,
        INET = PF_INET,
        INET6 = PF_INET6
    };

    class SocketAddress {
    public:
        /**
         * Do a lookup for the host.
         * This function uses getaddrinfo(3)
         *
         * Note this function will only return address in the PF_INET and
         * PF_INET6 family.
         *
         * The results of the lookup will be pushed onto the vector
         * passed as results.
         *
         * \param hostname null terminated string of the hostname to lookup.
         * May be null. If null returns addresses suitable for listening on.
         * \param port null terminated string for the service name or port number
         * to use in the returned addresses. May be null. If null the addresses
         * returned have an unspecified port which must be set. \see Port
         * \param socktype the type of socket the address should be limited to.
         * May be 0 for unspecified or all addresses available. see socket(2)
         * \param family the socket family to filter by use FAM_UNSPEC to get all.
         * \return 0 on success otherwise see man pages for getaddrinfo
         */
        static int Lookup(const char* const hostname, const char* const port,
                SockFamily family, std::vector<SocketAddress> &results);

        /**
         * Create a new SocketAddress with family PF_INET pointing to
         * any address for binding with an unspecified port and a type of STREAM.
         */
        SocketAddress();
        /**
         * Create a new SocketAddress with family PF_INET pointing to
         * any address for binding with the specified port and type STREAM.
         */
        SocketAddress(unsigned port);
        SocketAddress(addrinfo* ai);
        SocketAddress(const char* const name, SockFamily family);
        ~SocketAddress();

        /**
         * \return the port number for this address or -1 if
         * this address family does not support the idea of ports.
         */
        unsigned Port(void) const;

        /**
         * Set the port for this address. Does nothing if the
         * address family does not support the idea of ports.
         * \param p the new port number
         */
        void Port(unsigned p);

        /**
         * Some possible family types are: PF_UNIX, PF_INET, and PF_INET6.
         * (also called domains see man socket(2))
         * You cannot directly change an addresses family.
         * \return the family type of the address.
         */
        SockFamily Family(void) const { return (SockFamily)address.storage.ss_family; }

       /**
         * \return the address as a string if available.
         */
        std::string StringAddress(void) const;

        operator sockaddr* () { return &(address.addr); }
        operator socklen_t* () { return &length; }
        operator socklen_t () { return length; }

    private:
        sa_family_t &Family(void) { return address.storage.ss_family; }

        socklen_t length;

        union address_ {
            sockaddr addr;
            sockaddr_in in;
            sockaddr_in6 in6;
            //sockaddr_un un;
            sockaddr_storage storage;
        } address;
        std::string name;
    };
}

#endif

