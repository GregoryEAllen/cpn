
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

