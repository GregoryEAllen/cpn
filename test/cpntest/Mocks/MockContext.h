
#ifndef MOCKCONTEXT_H
#define MOCKCONTEXT_H

#include "Context.h"
class MockContext : public CPN::Context {
public:
    MockContext();
    virtual ~MockContext();
    virtual int LogLevel() const { return 0; }
    virtual int LogLevel(int level) { return level; }
    virtual void Log(int level, const std::string &msg) {}

    virtual CPN::Key_t SetupKernel(const std::string &name, const std::string &hostname,
            const std::string &servname, CPN::KernelBase *) { return 0; }
    virtual CPN::Key_t SetupKernel(const std::string &name, CPN::KernelBase*) { return 0; }
    virtual CPN::Key_t GetKernelKey(const std::string &kernel) { return 0; }
    virtual std::string GetKernelName(CPN::Key_t kernelkey) { return blank; }
    virtual void GetKernelConnectionInfo(CPN::Key_t kernelkey, std::string &hostname, std::string &servname) { }
    virtual void SignalKernelEnd(CPN::Key_t kernelkey) { }
    virtual CPN::Key_t WaitForKernelStart(const std::string &kernel) { return 0; }
    virtual void SignalKernelStart(CPN::Key_t kernelkey) { }

    virtual void SendCreateWriter(CPN::Key_t kernelkey, const CPN::SimpleQueueAttr &attr) { }
    virtual void SendCreateReader(CPN::Key_t kernelkey, const CPN::SimpleQueueAttr &attr) { }
    virtual void SendCreateQueue(CPN::Key_t kernelkey, const CPN::SimpleQueueAttr &attr) { }
    virtual void SendCreateNode(CPN::Key_t kernelkey, const CPN::NodeAttr &attr) { }

    virtual CPN::Key_t CreateNodeKey(CPN::Key_t kernelkey, const std::string &nodename) { return 0; }
    virtual CPN::Key_t GetNodeKey(const std::string &nodename) { return 0; }
    virtual std::string GetNodeName(CPN::Key_t nodekey) { return blank; }
    virtual CPN::Key_t GetNodeKernel(CPN::Key_t nodekey) { return 0; }
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
    virtual CPN::Key_t GetReaderKernel(CPN::Key_t portkey) { return 0; }
    virtual std::string GetReaderName(CPN::Key_t portkey) { return blank; }

    virtual CPN::Key_t GetCreateWriterKey(CPN::Key_t nodekey, const std::string &portname) { return 0; }
    virtual CPN::Key_t GetWriterNode(CPN::Key_t portkey) { return 0; }
    virtual CPN::Key_t GetWriterKernel(CPN::Key_t portkey) { return 0; }
    virtual std::string GetWriterName(CPN::Key_t portkey) { return blank; }

    virtual void ConnectEndpoints(CPN::Key_t writerkey, CPN::Key_t readerkey, const std::string &) { }
    virtual CPN::Key_t GetReadersWriter(CPN::Key_t readerkey) { return 0; }
    virtual CPN::Key_t GetWritersReader(CPN::Key_t writerkey) { return 0; }

    virtual void Terminate() {}
    virtual bool IsTerminated() { return false; }
private:
    std::string blank;
};
#endif
