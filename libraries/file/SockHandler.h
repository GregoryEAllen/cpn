#pragma once
#include "FileHandler.h"
#include "SocketAddress.h"

class SockHandler : public FileHandler {
public:

    static void CreatePair(SockHandler &sock1, SockHandler &sock2);

    void Connect(const SocketAddress &addr);

    void Connect(const SockAddrList &addrs);

    void ShutdownRead();
    void ShutdownWrite();
    // If block true then wait for buffer to be full
    // if block false don't wait for anything
    unsigned Recv(void *ptr, unsigned len, bool block);

    /** Convenience structure for Send
     * so you can do things like
     * Send(ptr, len, SendOpts().Block(false).NoSignal(true));
     * Send(ptr, len, SendOpts()) is exactly the same as
     * Write(ptr, len)
     */
    struct SendOpts {
        SendOpts() : flags(0) {}
        SendOpts(int f) : flags(f) {}
        SendOpts &Block(bool block);
        SendOpts &NoSignal(bool sig);
        SendOpts &More(bool more);
        int flags;
    };

    unsigned Send(const void *ptr, unsigned len, const SendOpts &opts);
private:
    bool Connect(const SocketAddress &addr, int &error);
};

