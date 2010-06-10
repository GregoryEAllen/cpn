
#ifndef MOCKDATABASE_H
#define MOCKDATABASE_H

#include "Database.h"
class MockDatabase : public CPN::Database {
public:
    MockDatabase();
    virtual ~MockDatabase();
    virtual int LogLevel() const { return 0; }
    virtual int LogLevel(int level) { return level; }
    virtual void Log(int level, const std::string &msg) {}

    virtual CPN::Key_t SetupHost(const std::string &name, const std::string &hostname,
            const std::string &servname, CPN::KernelBase *kmh) { return 0; }
    virtual CPN::Key_t SetupHost(const std::string &name, CPN::KernelBase *kmh) { return 0; }
    virtual CPN::Key_t GetHostKey(const std::string &host) { return 0; }
    virtual std::string GetHostName(CPN::Key_t hostkey) { return blank; }
    virtual void GetHostConnectionInfo(CPN::Key_t hostkey, std::string &hostname, std::string &servname) { }
    virtual void DestroyHostKey(CPN::Key_t hostkey) { }
    virtual CPN::Key_t WaitForHostStart(const std::string &host) { return 0; }
    virtual void SignalHostStart(CPN::Key_t hostkey) { }

    virtual void SendCreateWriter(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr) { }
    virtual void SendCreateReader(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr) { }
    virtual void SendCreateQueue(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr) { }
    virtual void SendCreateNode(CPN::Key_t hostkey, const CPN::NodeAttr &attr) { }

    virtual CPN::Key_t CreateNodeKey(CPN::Key_t hostkey, const std::string &nodename) { return 0; }
    virtual CPN::Key_t GetNodeKey(const std::string &nodename) { return 0; }
    virtual std::string GetNodeName(CPN::Key_t nodekey) { return blank; }
    virtual CPN::Key_t GetNodeHost(CPN::Key_t nodekey) { return 0; }
    virtual void SignalNodeStart(CPN::Key_t nodekey) { }
    virtual void SignalNodeEnd(CPN::Key_t nodekey) { }

    /** Waits until the node starts and returns the key, if the node is
     * already started returns the key
     */
    virtual CPN::Key_t WaitForNodeStart(const std::string &nodename) { return 0; }
    virtual void WaitForNodeEnd(const std::string &nodename) { }
    virtual void WaitForAllNodeEnd() { }


    virtual CPN::Key_t GetCreateReaderKey(CPN::Key_t nodekey, const std::string &portname) { return 0; }
    virtual CPN::Key_t GetReaderNode(CPN::Key_t portkey) { return 0; }
    virtual CPN::Key_t GetReaderHost(CPN::Key_t portkey) { return 0; }
    virtual std::string GetReaderName(CPN::Key_t portkey) { return blank; }
    virtual void DestroyReaderKey(CPN::Key_t portkey) { }

    virtual CPN::Key_t GetCreateWriterKey(CPN::Key_t nodekey, const std::string &portname) { return 0; }
    virtual CPN::Key_t GetWriterNode(CPN::Key_t portkey) { return 0; }
    virtual CPN::Key_t GetWriterHost(CPN::Key_t portkey) { return 0; }
    virtual std::string GetWriterName(CPN::Key_t portkey) { return blank; }
    virtual void DestroyWriterKey(CPN::Key_t portkey) { }

    virtual void ConnectEndpoints(CPN::Key_t writerkey, CPN::Key_t readerkey) { }
    virtual CPN::Key_t GetReadersWriter(CPN::Key_t readerkey) { return 0; }
    virtual CPN::Key_t GetWritersReader(CPN::Key_t writerkey) { return 0; }

    virtual void Terminate() {}
    virtual bool IsTerminated() { return false; }
private:
    std::string blank;
};
#endif
