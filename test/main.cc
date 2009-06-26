
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

/**
 * Code from http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html
 */
int main(int argc, char **argv) {
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	CppUnit::Test* suite = registry.makeTest();
	runner.addTest(suite);
	bool wasSuccessful = runner.run("", false);
	return wasSuccessful;
}

