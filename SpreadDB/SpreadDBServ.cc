
#include "SpreadDBServ.h"
#include "sp.h"
#include <cstdio>

SpreadDBServ::SpreadDBServ(const std::string &spread_name, const std::string &self,
        const std::string &cgroup)
: sclient(spread_name, self), clientgroup(cgroup)
{
}

void SpreadDBServ::Run() {
    while (true) {
        SpreadMessage msg;
        int ret = sclient.Recv(msg);
        if (ret < 0) {
            SP_error(ret);
            return;
        }
        Variant message = Variant::FromJSON(msg.Data());
        if (message.IsArray()) {
            if (message[0].AsString() == "command") {
                printf("<<< %s: %s\n", msg.Sender().c_str(), message[1].AsJSON().c_str());
                DispatchMessage(msg.Sender(), message[1]);
            }
        }
    }
}

void SpreadDBServ::SendMessage(const std::string &recipient, const Variant &msg) {
    printf(">>> %s: %s\n", recipient.c_str(), msg.AsJSON().c_str());
    SpreadMessage smsg(recipient, SpreadMessage::FIFO);
    smsg.SelfDiscard(true);
    msg.AsJSON(smsg.Data());
    int ret = sclient.Send(smsg);
    if (0 > ret) {
        SP_error(ret);
    }
}

void SpreadDBServ::BroadcastMessage(const Variant &msg) {
    SendMessage(clientgroup, msg);
}
