

#include "AutoBufferTest.h"
#include <cppunit/TestAssert.h>
#include "AutoCircleBuffer.h"
#include <cstdlib>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( AutoBufferTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, ## __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

void AutoBufferTest::setUp() {}

void AutoBufferTest::tearDown() {}

void AutoBufferTest::CircleBufferTest1() {
    AutoCircleBuffer buff;
    char in = 0;
    char out = 0;
    for (int i = 0; i < 1000; ++i) {
        in = (char)i;
        buff.Put(&in, sizeof(in));
        buff.Get(&out, sizeof(out));
        CPPUNIT_ASSERT(in == out);
    }
}

void AutoBufferTest::CircleBufferTest2() {
    AutoCircleBuffer buff;
    srand(1);
    int in[10] = {0};
    int out[10] = {0};
    for (int i = 0; i < 1000; ++i) {
        DEBUG("Sending: {\n");
        for (unsigned j = 0; j < sizeof(in)/sizeof(in[0]); ++j) {
            in[j] = rand();
            DEBUG("\t%d,\n", in[j]);
        }
        DEBUG("}\n");
        buff.Put((char*)&in[0], sizeof(in));
        CPPUNIT_ASSERT(buff.Get((char*)&out[0], sizeof(out)));
        DEBUG("Recieved: {\n");
        for (unsigned j = 0; j < sizeof(out)/sizeof(out[0]); ++j) {
            DEBUG("\t%d,\n", out[j]);
            CPPUNIT_ASSERT(in[j] == out[j]);
        }
        DEBUG("}\n");
    }
}
