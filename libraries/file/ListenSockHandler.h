
#pragma once

#include "FileHandler.h"
#include "SocketAddress.h"

class ListenSockHandler : public FileHandler {
public:

    void Listen(const SocketAddress &addr);

    void Listen(const SockAddrList &addrs);

    /**
     * \return -1 if no connection >=0 on success
     * \throws ErrnoException for errors
     */
    int Accept(SocketAddress &addr);

    virtual void OnRead() = 0;
    virtual void OnError() = 0;
    virtual void OnInval() = 0;
private:
    // These can't happen
    void OnWrite() {}
    void OnHup() {}
    bool Listen(const SocketAddress &addr, int &error);
};

