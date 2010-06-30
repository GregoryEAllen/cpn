#include "NodeBase.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "QueueWriterAdapter.h"
#include <string>
#include <unistd.h>

using namespace CPN;
using std::string;

class FileReadNode : public NodeBase {
public:
    FileReadNode(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr), fd(-1), output("output")
    {
        JSONToVariant p;
        p.Parse(attr.GetParam());
        ASSERT(p.Done(), "Error parsing param line %u column %u", p.GetLine(), p.GetColumn());
        Variant param = p.Get();
        ASSERT(!param["fd"].IsNull(), "FileReadNode must have fd parameter");
        fd = param["fd"].AsInt();
        if (!param["outport"].IsNull()) {
            output = param["outport"].AsString();
        }
    }
private:
    void Process();
    int fd;
    string output;
};

CPN_DECLARE_NODE_FACTORY(FileReadNode, FileReadNode);

void FileReadNode::Process() {
    QueueWriterAdapter<void> out = GetWriter(output);
    try {
        while (true) {
            unsigned blocksize = out.MaxThreshold();
            void *ptr = out.GetEnqueuePtr(blocksize);
            unsigned numread = read(fd, ptr, blocksize);
            if (numread < 1) {
                // Do something if error??
                break;
            }
            out.Enqueue(numread);
        }
    } catch (...) {
        close(fd);
    }
    close(fd);
}


