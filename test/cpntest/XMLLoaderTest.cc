
#include "XMLLoaderTest.h"
#include "KernelXMLLoader.h"
#include "MockNodeFactory.h"
#include <cppunit/TestAssert.h>

CPPUNIT_TEST_SUITE_REGISTRATION( XMLLoaderTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

void XMLLoaderTest::setUp(void) {
	CPNRegisterNodeFactory(MockNodeFactory::GetInstance());
}

void XMLLoaderTest::tearDown(void) {
}

void XMLLoaderTest::test1(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	CPN::KernelXMLLoader loader("XMLLoaderTest.xml");
	CPN::Kernel kernel(loader.GetKernelAttr());
	loader.SetupNodes(kernel);
	loader.SetupQueues(kernel);
	kernel.Start();
	kernel.Wait();
}

