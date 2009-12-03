
#pragma once

#include "SpreadMessage.h"
#include <string>

class SpreadClient {
public:
    SpreadClient(
            const std::string &spread_name,
            const std::string &name
            );

    ~SpreadClient();

    const std::string &GetPrivateGroup() const { return private_group; }

    int Mailbox() const { return mbox; }

    void Join(const std::string &group_name);

    void Leave(const std::string &group_name);

    void Send(const SpreadMessage &msg);

    SpreadMessage Recv();

private:
    int mbox;
    std::string private_group;
};

