
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cstdio>

/**
 * Code from http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html
 */
int main(int argc, char **argv) {
	printf("Start testing...\n");
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	CppUnit::Test* suite = registry.makeTest();
	runner.addTest(suite);
	printf("Running tests...\n");
	bool wasSuccessful = runner.run("", false);
	return wasSuccessful;
}

