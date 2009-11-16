
#include "MockSyncNode.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "Assert.h"


MockSyncNode::MockSyncNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    param = *(Param*)attr.GetArg().GetBuffer();
}

void MockSyncNode::Process() {
    switch (param.mode) {
    case MODE_SOURCE:
        {
            kernel.WaitNodeStart(param.othernode);
            CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
            qattr.SetReader(param.othernode, "x").SetWriter(GetName(), "y");
            qattr.SetDatatype<unsigned long>();
            kernel.CreateQueue(qattr);
            CPN::QueueWriterAdapter<unsigned long> out = GetWriter("y");
            unsigned long val = 1;
            out.Enqueue(&val, 1);
            //out.Release();
        }
        break;
    case MODE_SINK:
        {
            kernel.WaitNodeTerminate(param.othernode);
            CPN::QueueReaderAdapter<unsigned long> in = GetReader("x");
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


void MockSyncNode::RegisterType() {
    CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory>(new MockSyncNodeFactory));
}

void SyncSource::Run1(CPN::NodeBase *nb) {
    nb->GetKernel()->WaitNodeStart(othernode);
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    qattr.SetDatatype<unsigned long>();
    nb->GetKernel()->CreateQueue(qattr);
    CPN::QueueWriterAdapter<unsigned long> out = nb->GetWriter("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
}
void SyncSource::Run2(CPN::NodeBase *nb) {
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    qattr.SetDatatype<unsigned long>();
    nb->GetKernel()->CreateQueue(qattr);
    CPN::QueueWriterAdapter<unsigned long> out = nb->GetWriter("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
}
void SyncSource::Run3(CPN::NodeBase *nb) {
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    qattr.SetDatatype<unsigned long>();
    nb->GetKernel()->CreateQueue(qattr);
    CPN::QueueWriterAdapter<unsigned long> out = nb->GetWriter("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
    out.Release();
}
void SyncSource::Run4(CPN::NodeBase *nb) {
    nb->GetKernel()->WaitNodeStart(othernode);
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader(othernode, "x").SetWriter(nb->GetName(), "y");
    nb->GetKernel()->CreateQueue(qattr);
    CPN::QueueWriterAdapter<unsigned long> out = nb->GetWriter("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
    out.Release();
}
// goes only with SyncSink::Run3
void SyncSource::Run5(CPN::NodeBase *nb) {
    CPN::QueueWriterAdapter<unsigned long> out = nb->GetWriter("y");
    unsigned long val = 1;
    out.Enqueue(&val, 1);
    out.Release();
}

void SyncSink::Run1(CPN::NodeBase *nb) {
    nb->GetKernel()->WaitNodeTerminate(othernode);
    CPN::QueueReaderAdapter<unsigned long> in = nb->GetReader("x");
    unsigned long val;
    ASSERT(in.Dequeue(&val, 1));
    ASSERT(val == 1);
    ASSERT(!in.Dequeue(&val, 1));
}
void SyncSink::Run2(CPN::NodeBase *nb) {
    CPN::QueueReaderAdapter<unsigned long> in = nb->GetReader("x");
    unsigned long val;
    ASSERT(in.Dequeue(&val, 1));
    ASSERT(val == 1);
    ASSERT(!in.Dequeue(&val, 1));
}
// goes only with SyncSource::Run5
void SyncSink::Run3(CPN::NodeBase *nb) {
    CPN::QueueAttr qattr(sizeof(unsigned long), sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetWriter(othernode, "y").SetReader(nb->GetName(), "x");
    nb->GetKernel()->CreateQueue(qattr);
    CPN::QueueReaderAdapter<unsigned long> in = nb->GetReader("x");
    unsigned long val;
    ASSERT(in.Dequeue(&val, 1));
    ASSERT(val == 1);
    ASSERT(!in.Dequeue(&val, 1));
}
