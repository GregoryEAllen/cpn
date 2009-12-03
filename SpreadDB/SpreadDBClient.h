
#pragma once

#include "RemoteDBClient.h"
#include "SpreadClient.h"
#include "Pthread.h"

class SpreadDBClient : public CPN::RemoteDBClient, public Pthread {
    public:
        SpreadDBClient(const std::string &spread_name, const std::string &self,
                const std::string &targetgroup, const std::string &cgroup);
        ~SpreadDBClient();

        int LogLevel() const;
        int LogLevel(int level);
        void Log(int level, const std::string &msg) const;

        void Shutdown();
    private:
        void SendMessage(const Variant &msg);
        void *EntryPoint();

        SpreadClient sclient;
        const std::string group;
        const std::string clientgroup;
        int loglevel;

};

