
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

    int Join(const std::string &group_name);

    int Leave(const std::string &group_name);

    int Send(const SpreadMessage &msg);

    int Recv(SpreadMessage &msg);

    void Close();
private:
    int mbox;
    std::string private_group;
};

