
#include "ProducerNode.h"
#include "ConsumerNode.h"
#include "ThresholdQueue.h"
#include "Kernel.h"

int main(int argc, char **argv) {
	CPN::Kernel kern(CPN::KernelAttr(1, "kernel"));
	CPN::ThresholdQueue q(CPN::QueueAttr(2, "q"), 10, 5);
	ProducerNode A(kern, CPN::NodeAttr(3, "A"));
	ConsumerNode B(kern, CPN::NodeAttr(4, "B"));
	A.ConnectWriter("out", q.GetWriter());
	B.ConnectReader("in", q.GetReader());
	A.Start();
	B.Start();
	A.Join();
	B.Join();
	return 0;
}

