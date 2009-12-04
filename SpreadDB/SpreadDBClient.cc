
#include "SpreadDBClient.h"
#include "sp.h"

SpreadDBClient::SpreadDBClient(const std::string &spread_name, const std::string &self,
       const std::string &targetgroup, const std::string &cgroup)
    : sclient(spread_name, self), group(targetgroup), clientgroup(cgroup), loglevel(Logger::WARNING)
{
    sclient.Join(clientgroup);
}

SpreadDBClient::~SpreadDBClient() {
    Shutdown();
}

int SpreadDBClient::LogLevel() const {
    PthreadMutexProtected mp(lock);
    return loglevel;
}

int SpreadDBClient::LogLevel(int level) {
    PthreadMutexProtected mp(lock);
    return loglevel = level;
}

void SpreadDBClient::Log(int level, const std::string &msg) const {
    PthreadMutexProtected mp(lock);
    if (level >= loglevel) {
        SpreadMessage smsg(group, SpreadMessage::FIFO);
        smsg.SelfDiscard(true);
        Variant message(Variant::ArrayType);
        message[0] = "log";
        message[1] = msg;
        message.AsJSON(smsg.Data());
        sclient.Send(smsg);
    }
}

void SpreadDBClient::Shutdown() {
    PthreadMutexProtected mp(lock);
    sclient.Close();
}

void SpreadDBClient::SendMessage(const Variant &msg) {
    SpreadMessage smsg(group, SpreadMessage::FIFO);
    smsg.SelfDiscard(true);
    Variant message(Variant::ArrayType);
    message[0] = "command";
    message[1] = msg;
    message.AsJSON(smsg.Data());
    printf(">>> ");
    fwrite(&smsg.Data()[0], 1, smsg.Data().size(), stdout);
    printf("\n");
    sclient.Send(smsg);
}

void *SpreadDBClient::EntryPoint() {
    while (true) {
        SpreadMessage smsg;
        int ret = sclient.Recv(smsg);
        if (ret < 0) {
            SP_error(ret);
            return 0;
        }
        printf("<<< ");
        fwrite(&smsg.Data()[0], 1, smsg.Data().size(), stdout);
        printf("\n");
        Variant message = Variant::FromJSON(smsg.Data());
        DispatchMessage(message);
    }
    return 0;
}

