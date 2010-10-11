
/**
 */
#ifndef TESTTEMPLATE_H
#define TESTTEMPLATE_H
#include <cppunit/extensions/HelperMacros.h>
class NodeFactoryTest : public CppUnit::TestFixture {
public:
    void setUp(void);

    void tearDown(void);

    CPPUNIT_TEST_SUITE( NodeFactoryTest );
    CPPUNIT_TEST( TestFactoryStore );
    CPPUNIT_TEST( TestInvalidName );
    CPPUNIT_TEST( TestCleanUp );
    CPPUNIT_TEST_SUITE_END();

    void TestFactoryStore(void);
    void TestInvalidName(void);
    void TestCleanUp(void);
};
#endif
