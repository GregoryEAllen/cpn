
#include "RemoteDBTest.h"
#include <cppunit/TestAssert.h>
#include "RemoteDBClient.h"
#include "RemoteDBServer.h"
#include "AutoUnlock.h"

#include "Pthread.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "PthreadFunctional.h"

#include <deque>
#include <map>

CPPUNIT_TEST_SUITE_REGISTRATION( RemoteDBTest);

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::Key_t;


class LocalRDBServ : public CPN::RemoteDBServer, public Pthread {
public:

    LocalRDBServ()
        : die(false)
    {
    }
    ~LocalRDBServ() {
        PthreadMutexProtected al(lock);
        die = true;
        cond.Signal();
    }
    void EnqueueMessage(const std::string &name, const Variant &msg) {
        PthreadMutexProtected al(lock);
        msgqueue.push_back(std::make_pair(name, msg));
        cond.Signal();
    }

    void Register(const std::string &name, CPN::RemoteDBClient *rdbc) {
        PthreadMutexProtected al(lock);
        replymap[name] = rdbc;
    }
    void UnRegister(const std::string &name) {
        PthreadMutexProtected al(lock);
        replymap.erase(name);
    }

    void SendMessage(const std::string &recipient, const Variant &msg) {
        DEBUG("Reply %s -> %s\n", recipient.c_str(), msg.AsJSON().c_str());
        replymap[recipient]->DispatchMessage(msg);
    }

    void BroadcastMessage(const Variant &msg) {
        DEBUG("Broadcast %s\n", msg.AsJSON().c_str());
        for(std::map<std::string, CPN::RemoteDBClient *>::iterator entry = replymap.begin();
                entry != replymap.end(); ++entry)
        {
            (entry->second)->DispatchMessage(msg);
        }
    }

    void *EntryPoint() {
        PthreadMutexProtected al(lock);
        while (!die) {
            if (msgqueue.empty()) {
                cond.Wait(lock);
            } else {
                std::pair<std::string, Variant> entry = msgqueue.front();
                msgqueue.pop_front();
                DEBUG("Processing %s -> %s\n", entry.first.c_str(), entry.second.AsJSON().c_str());
                DispatchMessage(entry.first, entry.second);
            }
        }
        return 0;
    }
private:
    bool die;
    std::map<std::string, CPN::RemoteDBClient *> replymap;
    std::deque<std::pair<std::string, Variant> > msgqueue;
    PthreadCondition cond;
    PthreadMutex lock;
};

class LocalRDBClient : public CPN::RemoteDBClient {
public:
    LocalRDBClient(LocalRDBServ *lrdbs_, const std::string &name_)
        : lrdbs(lrdbs_), name(name_)
    {
        lrdbs->Register(name, this);
    }
    ~LocalRDBClient() {
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
    LocalRDBServ *lrdbs;
    std::string name;
};

void RemoteDBTest::setUp() {
    serv = new LocalRDBServ;
    serv->Start();
}

void RemoteDBTest::tearDown() {
    delete serv;
    serv = 0;
}

void RemoteDBTest::HostSetupTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
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
    lrdbc.DestroyHostKey(hostkey);
}

void RemoteDBTest::WaitForHostTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
    std::string hostname = "bogus1";
    std::string servname = "bogus2";
    std::string name = "bogus3";
    m_hostname = name;
    Pthread *waiter = CreatePthreadFunctional(this, &RemoteDBTest::WaitForHostSetup);
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

void *RemoteDBTest::WaitForHostSetup() {
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
    lock.Lock();
    signaled = true;
    cond.Signal();
    lock.Unlock();
    m_hostkey = lrdbc.WaitForHostStart(m_hostname);
    ASSERT(m_hostkey > 0);
    return 0;
}

void RemoteDBTest::CreateNodeTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
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

void RemoteDBTest::WaitForNodeTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
    m_nodename = "bogus node";
    Pthread *waiter = CreatePthreadFunctional(this, &RemoteDBTest::WaitForNode);
    waiter->Start();
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

void *RemoteDBTest::WaitForNode() {
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
    lock.Lock();
    signaled = true;
    cond.Signal();
    lock.Unlock();
    lrdbc.WaitForNodeStart(m_nodename);
    lrdbc.WaitForNodeEnd(m_nodename);
    return 0;
}

void RemoteDBTest::ReaderTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);

    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(4321, m_nodename);

    Key_t rkey = lrdbc.GetCreateReaderKey(m_nodekey, "bogus key");
    CPPUNIT_ASSERT(rkey > 0);
    CPPUNIT_ASSERT_EQUAL(m_nodekey, lrdbc.GetReaderNode(rkey));
    CPPUNIT_ASSERT_EQUAL((Key_t)4321, lrdbc.GetReaderHost(rkey));
    CPPUNIT_ASSERT_EQUAL(std::string("bogus key"), lrdbc.GetReaderName(rkey));
    lrdbc.DestroyReaderKey(rkey);
}

void RemoteDBTest::WriterTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);

    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(4321, m_nodename);

    Key_t wkey = lrdbc.GetCreateWriterKey(m_nodekey, "bogus key");
    CPPUNIT_ASSERT(wkey > 0);
    CPPUNIT_ASSERT_EQUAL(m_nodekey, lrdbc.GetWriterNode(wkey));
    CPPUNIT_ASSERT_EQUAL((Key_t)4321, lrdbc.GetWriterHost(wkey));
    CPPUNIT_ASSERT_EQUAL(std::string("bogus key"), lrdbc.GetWriterName(wkey));
    lrdbc.DestroyWriterKey(wkey);
}

void RemoteDBTest::ConnectTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    LocalRDBClient lrdbc(serv, __PRETTY_FUNCTION__);
    m_nodename = "bogus node";
    m_nodekey = lrdbc.CreateNodeKey(4321, m_nodename);

    Key_t wkey = lrdbc.GetCreateWriterKey(m_nodekey, "bogus writer");
    Key_t rkey = lrdbc.GetCreateReaderKey(m_nodekey, "bogus reader");
    lrdbc.ConnectEndpoints(wkey, rkey);

    CPPUNIT_ASSERT_EQUAL(wkey, lrdbc.GetReadersWriter(rkey));
    CPPUNIT_ASSERT_EQUAL(rkey, lrdbc.GetWritersReader(wkey));
}

void RemoteDBTest::CreateWriter(CPN::Key_t dst, const CPN::SimpleQueueAttr &attr) {
}

void RemoteDBTest::CreateReader(CPN::Key_t dst, const CPN::SimpleQueueAttr &attr) {
}

void RemoteDBTest::CreateQueue(CPN::Key_t dst, const CPN::SimpleQueueAttr &attr) {
}

void RemoteDBTest::CreateNode(CPN::Key_t dst, const CPN::NodeAttr &attr) {
}

