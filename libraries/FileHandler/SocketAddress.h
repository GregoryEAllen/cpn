
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

    SocketAddress();
    SocketAddress(addrinfo *info);
    SocketAddress(addrinfo *info, unsigned portnum);
    /**
     * Create a SocketAddress from the underlying sockaddr and length
     * \param addr pointer tot he address structure
     * \param length the length of the address structure
     */
    SocketAddress(sockaddr *addr, socklen_t len);
    ~SocketAddress() {}

    std::string GetHostName(bool numerichost = false) const;
    std::string GetServName() const;
    unsigned GetServ() const;

    Type_t GetType() const;


    sockaddr *GetAddr() { return &address.addr; }
    socklen_t &GetLen() { return length; }
    sa_family_t &Family() { return address.storage.ss_family; }
    sa_family_t Family() const { return address.storage.ss_family; }

    void SetFromSockName(int fd);
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


