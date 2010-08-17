
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

const char VALID_OPS[] = "pr";

/**
 * Code from http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html
 */
int main(int argc, char **argv) {
    // Ignore sigpipe
	signal(SIGPIPE,SIG_IGN);
    bool pause = false;
    bool repeat = false;
    bool procOpts = true;
    while (procOpts) {
        int opt = getopt(argc, argv, VALID_OPS);
        switch (opt) {
        case 'p':
            pause = true;
            break;
        case 'r':
            repeat = true;
            break;
        case -1:
            procOpts = false;
            break;
        default:
            printf("Unknown param\n");
            return 0;
        }
    }
    std::string testname = "";
    if (argc > optind) {
        testname = argv[optind];
    }

    if (pause) {
        printf("Press enter to continue.\n");
        getchar();
    }
    bool wasSuccessful;
    do {
        CppUnit::TextUi::TestRunner runner;
        CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
        CppUnit::Test* suite = registry.makeTest();
        runner.addTest(suite);
        wasSuccessful = runner.run(testname, false);
    } while (repeat && wasSuccessful);

    return !wasSuccessful;
}

