
#include "XMLLoaderTest.h"
#include "KernelXMLLoader.h"
#include <cppunit/TestAssert.h>

CPPUNIT_TEST_SUITE_REGISTRATION( XMLLoaderTest );


void XMLLoaderTest::setUp(void) {
}

void XMLLoaderTest::tearDown(void) {
}

void XMLLoaderTest::test1(void) {
	CPN::KernelXMLLoader loader("xmlcpn/xml/ExampleHostprog_zeus.xml");
	CPN::Kernel kernel(loader.GetKernelAttr());
	loader.SetupNodes(kernel);
	loader.SetupQueues(kernel);
}

