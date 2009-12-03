
#pragma once
#include "RemoteDBServ.h"
#include "SpreadClient.h"

class SpreadDBServ : public CPN::RemoteDBServ {
public:
    SpreadDBServ(const std::string &spread_name, const std::string &self,
            const std::string &cgroup);
    void Run();
private:

    void SendMessage(const std::string &recipient, const Variant &msg);
    void BroadcastMessage(const Variant &msg);
    SpreadClient sclient;
    std::string clientgroup;
};

