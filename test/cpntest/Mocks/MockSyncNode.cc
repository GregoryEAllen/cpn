
#include "MockSyncNode.h"
#include "OQueue.h"
#include "IQueue.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "ThrowingAssert.h"


MockSyncNode::MockSyncNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
}

void MockSyncNode::Process() {
    Mode_t mode = GetParam<Mode_t>("mode");
    std::string othernode = GetParam("other");
    switch (mode) {
    case MODE_SOURCE:
        {
            kernel.WaitNodeStart(othernode);
            CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
            qattr.SetReader(othernode, "x").SetWriter(GetName(), "y");
            qattr.SetDatatype<unsigned long>();
            kernel.CreateQueue(qattr);
            CPN::OQueue<unsigned long> out = GetOQueue("y");
            unsigned long val = 1;
            out.Enqueue(&val, 1);
            //out.Release();
        }
        break;
    case MODE_SINK:
        {
            kernel.WaitNodeTerminate(othernode);
            CPN::IQueue<unsigned long> in = GetIQueue("x");
            unsigned long val;
            ASSERT(in.Dequeue(&val, 1));
            ASSERT(val == 1);
            ASSERT(!in.Dequeue(&val, 1));
        }
        break;
    default:
        ASSERT(false);
    }
}

class MockSyncNodeFactory : public CPN::NodeFactory {
public:
    MockSyncNodeFactory() : CPN::NodeFactory(MOCKSYNCNODE_TYPENAME) {}

    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
        return CPN::shared_ptr<CPN::NodeBase>(new MockSyncNode(ker, attr));
    }

private:
};


CPN::shared_ptr<CPN::NodeFactory> cpninitmocksyncnodetypename(void) {
    return (CPN::shared_ptr<CPN::NodeFactory>(new MockSyncNodeFactory));
}

void SyncSource::Run1(CPN::NodeBase *nb, std::string othernode) {
    nb->GetKernel()->WaitNodeStart(othernode);
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    qattr.SetDatatype<unsigned long>();
    nb->GetKernel()->CreateQueue(qattr);
    CPN::OQueue<unsigned long> out = nb->GetOQueue("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
}
void SyncSource::Run2(CPN::NodeBase *nb, std::string othernode) {
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    qattr.SetDatatype<unsigned long>();
    nb->GetKernel()->CreateQueue(qattr);
    CPN::OQueue<unsigned long> out = nb->GetOQueue("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
}
void SyncSource::Run3(CPN::NodeBase *nb, std::string othernode) {
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    qattr.SetDatatype<unsigned long>();
    nb->GetKernel()->CreateQueue(qattr);
    CPN::OQueue<unsigned long> out = nb->GetOQueue("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
    out.Release();
}
void SyncSource::Run4(CPN::NodeBase *nb, std::string othernode) {
    nb->GetKernel()->WaitNodeStart(othernode);
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    nb->GetKernel()->CreateQueue(qattr);
    CPN::OQueue<unsigned long> out = nb->GetOQueue("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
    out.Release();
}
// goes only with SyncSink::Run3
void SyncSource::Run5(CPN::NodeBase *nb, std::string othernode) {
    CPN::OQueue<unsigned long> out = nb->GetOQueue("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
    out.Release();
}

void SyncSink::Run1(CPN::NodeBase *nb, std::string othernode) {
    nb->GetKernel()->WaitNodeTerminate(othernode);
    CPN::IQueue<unsigned long> in = nb->GetIQueue("x");
    unsigned long val;
    ASSERT(in.Dequeue(&val, 1));
    ASSERT(val == 1);
    ASSERT(!in.Dequeue(&val, 1));
}
void SyncSink::Run2(CPN::NodeBase *nb, std::string othernode) {
    CPN::IQueue<unsigned long> in = nb->GetIQueue("x");
    unsigned long val;
    ASSERT(in.Dequeue(&val, 1));
    ASSERT(val == 1);
    ASSERT(!in.Dequeue(&val, 1));
}
// goes only with SyncSource::Run5
void SyncSink::Run3(CPN::NodeBase *nb, std::string othernode) {
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetWriter(othernode, "y").SetReader(nb->GetName(), "x");
    nb->GetKernel()->CreateQueue(qattr);
    CPN::IQueue<unsigned long> in = nb->GetIQueue("x");
    unsigned long val;
    ASSERT(in.Dequeue(&val, 1));
    ASSERT(val == 1);
    ASSERT(!in.Dequeue(&val, 1));
}
