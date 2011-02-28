
#include "RemoteContextTest.h"
#include <cppunit/TestAssert.h>
#include "RemoteContextClient.h"
#include "RemoteContextServer.h"
#include "AutoUnlock.h"

#include "Pthread.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "PthreadFunctional.h"

#include <deque>
#include <map>

CPPUNIT_TEST_SUITE_REGISTRATION( RemoteContextTest);

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

#if 0
#define DBPRINT(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define DBPRINT(fmt, ...)
#endif

using CPN::shared_ptr;
using CPN::Key_t;


class LocalRContextServ : public CPN::RemoteContextServer, public Pthread {
public:

    LocalRContextServ()
        : die(false)
    {
    }
    ~LocalRContextServ() {
        {
            PthreadMutexProtected al(lock);
            die = true;
            cond.Signal();
        }
        Join();
    }
    void EnqueueMessage(const std::string &name, const Variant &msg) {
        PthreadMutexProtected al(lock);
        msgqueue.push_back(std::make_pair(name, msg.Copy()));
        cond.Signal();
    }

    void Register(const std::string &name, CPN::RemoteContextClient *rdbc) {
        PthreadMutexProtected al(lock);
        replymap[name] = rdbc;
    }
    void UnRegister(const std::string &name) {
        PthreadMutexProtected al(lock);
        replymap.erase(name);
    }

    void SendMessage(const std::string &recipient, const Variant &msg) {
        DBPRINT("Reply %s -> %s\n", recipient.c_str(), msg.AsJSON().c_str());
        replymap[recipient]->DispatchMessage(msg.Copy());
    }

    void BroadcastMessage(const Variant &msg) {
        DBPRINT("Broadcast %s\n", msg.AsJSON().c_str());
        for(std::map<std::string, CPN::RemoteContextClient *>::iterator entry = replymap.begin();
                entry != replymap.end(); ++entry)
        {
            (entry->second)->DispatchMessage(msg.Copy());
        }
    }

    void LogMessage(const std::string &msg) {
        DEBUG("log: %s\n", msg.c_str());
    }

    void *EntryPoint() {
        PthreadMutexProtected al(lock);
        while (!die) {
            if (msgqueue.empty()) {
                cond.Wait(lock);
            } else {
                std::pair<std::string, Variant> entry = msgqueue.front();
                msgqueue.pop_front();
                DBPRINT("Processing %s -> %s\n", entry.first.c_str(), entry.second.AsJSON().c_str());
                DispatchMessage(entry.first, entry.second.Copy());
            }
        }
        return 0;
    }
private:
    bool die;
    std::map<std::string, CPN::RemoteContextClient *> replymap;
    std::deque<std::pair<std::string, Variant> > msgqueue;
    PthreadCondition cond;
    PthreadMutex lock;
};

class LocalRContextClient : public CPN::RemoteContextClient {
public:
    LocalRContextClient(LocalRContextServ *lrdbs_, const std::string &name_)
        : lrdbs(lrdbs_), name(name_)
    {
        lrdbs->Register(name, this);
    }
    ~LocalRContextClient() {
        lrdbs->UnRegister(name);
    }

    int LogLevel() const {
        return 0;
    }

    int LogLevel(int level) {
        return level;
    }

    void Log(int level, const std::string &msg) const {
    }

protected:
    void SendMessage(const Variant &msg) {
        AutoUnlock<PthreadMutex> aul(lock);
        lrdbs->EnqueueMessage(name, msg);
    }
private:
    LocalRContextServ *lrdbs;
    std::string name;
};

void RemoteContextTest::setUp() {
    serv = new LocalRContextServ;
    serv->Start();
}

void RemoteContextTest::tearDown() {
    delete serv;
    serv = 0;
}

void RemoteContextTest::HostSetupTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    std::string hostname = "bogus1";
    std::string servname = "bogus2";
    std::string name = "bogus3";

    Key_t hostkey = lrdbc.SetupHost(name, hostname, servname, this);
    lrdbc.SignalHostStart(hostkey);
    CPPUNIT_ASSERT(hostkey > 0);
    CPPUNIT_ASSERT(lrdbc.GetHostKey(name) == hostkey);
    CPPUNIT_ASSERT(lrdbc.GetHostName(hostkey) == name);
    CPPUNIT_ASSERT(lrdbc.WaitForHostStart(name) == hostkey);
    std::string hname, sname;
    lrdbc.GetHostConnectionInfo(hostkey, hname, sname);
    CPPUNIT_ASSERT(hname == hostname);
    CPPUNIT_ASSERT(sname == servname);
    lrdbc.SignalHostEnd(hostkey);
}

void RemoteContextTest::WaitForHostTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    std::string hostname = "bogus1";
    std::string servname = "bogus2";
    std::string name = "bogus3";
    m_hostname = name;
    Pthread *waiter = CreatePthreadFunctional(this, &RemoteContextTest::WaitForHostSetup);
    CPPUNIT_ASSERT_EQUAL(0, waiter->Error());
    lock.Lock();
    signaled = false;
    waiter->Start();
    while (!signaled) {
        cond.Wait(lock);
    }
    lock.Unlock();
    Key_t hostkey = lrdbc.SetupHost(name, hostname, servname, this);
    CPPUNIT_ASSERT(hostkey > 0);
    lrdbc.SignalHostStart(hostkey);
    waiter->Join();
    delete waiter;
    CPPUNIT_ASSERT_EQUAL(m_hostkey, hostkey);
}

void *RemoteContextTest::WaitForHostSetup() {
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    lock.Lock();
    signaled = true;
    cond.Signal();
    lock.Unlock();
    m_hostkey = lrdbc.WaitForHostStart(m_hostname);
    ASSERT(m_hostkey > 0);
    return 0;
}

void RemoteContextTest::CreateNodeTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(1234, m_nodename);
    CPPUNIT_ASSERT(m_nodename > 0);
    CPPUNIT_ASSERT(lrdbc.GetNodeKey(m_nodename) == m_nodekey);
    CPPUNIT_ASSERT(lrdbc.GetNodeName(m_nodekey) == m_nodename);
    CPPUNIT_ASSERT(lrdbc.GetNodeHost(m_nodekey) == 1234);
    lrdbc.SignalNodeStart(m_nodekey);
    CPPUNIT_ASSERT(lrdbc.WaitForNodeStart(m_nodename) == m_nodekey);
    lrdbc.SignalNodeEnd(m_nodekey);
    lrdbc.WaitForNodeEnd(m_nodename);
}

void RemoteContextTest::WaitForNodeTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    m_nodename = "bogus node";
    Pthread *waiter = CreatePthreadFunctional(this, &RemoteContextTest::WaitForNode);
    CPPUNIT_ASSERT_EQUAL(0, waiter->Error());
    lock.Lock();
    signaled = false;
    waiter->Start();
    while (!signaled) {
        cond.Wait(lock);
    }
    signaled = false;
    lock.Unlock();
    m_nodekey = lrdbc.CreateNodeKey(1234, m_nodename);
    CPPUNIT_ASSERT(m_nodename > 0);
    lrdbc.SignalNodeStart(m_nodekey);
    lrdbc.SignalNodeEnd(m_nodekey);
    waiter->Join();
    lrdbc.WaitForAllNodeEnd();
    delete waiter;
}

void *RemoteContextTest::WaitForNode() {
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    lock.Lock();
    signaled = true;
    cond.Signal();
    lock.Unlock();
    lrdbc.WaitForNodeStart(m_nodename);
    lrdbc.WaitForNodeEnd(m_nodename);
    return 0;
}

void RemoteContextTest::ReaderTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);

    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(4321, m_nodename);

    Key_t rkey = lrdbc.GetCreateReaderKey(m_nodekey, "bogus key");
    CPPUNIT_ASSERT(rkey > 0);
    CPPUNIT_ASSERT_EQUAL(m_nodekey, lrdbc.GetReaderNode(rkey));
    CPPUNIT_ASSERT_EQUAL((Key_t)4321, lrdbc.GetReaderHost(rkey));
    CPPUNIT_ASSERT_EQUAL(std::string("bogus key"), lrdbc.GetReaderName(rkey));
}

void RemoteContextTest::WriterTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);

    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(4321, m_nodename);

    Key_t wkey = lrdbc.GetCreateWriterKey(m_nodekey, "bogus key");
    CPPUNIT_ASSERT(wkey > 0);
    CPPUNIT_ASSERT_EQUAL(m_nodekey, lrdbc.GetWriterNode(wkey));
    CPPUNIT_ASSERT_EQUAL((Key_t)4321, lrdbc.GetWriterHost(wkey));
    CPPUNIT_ASSERT_EQUAL(std::string("bogus key"), lrdbc.GetWriterName(wkey));
}

void RemoteContextTest::ConnectTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRContextClient lrdbc(serv, __PRETTY_FUNCTION__);
    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(4321, m_nodename);

    Key_t wkey = lrdbc.GetCreateWriterKey(m_nodekey, "bogus writer");
    Key_t rkey = lrdbc.GetCreateReaderKey(m_nodekey, "bogus reader");
    lrdbc.ConnectEndpoints(wkey, rkey, "name");

    CPPUNIT_ASSERT_EQUAL(wkey, lrdbc.GetReadersWriter(rkey));
    CPPUNIT_ASSERT_EQUAL(rkey, lrdbc.GetWritersReader(wkey));
}

