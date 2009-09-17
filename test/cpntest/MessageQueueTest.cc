
#include "MessageQueueTest.h"
#include "MessageQueue.h"
#include <cppunit/TestAssert.h>

CPPUNIT_TEST_SUITE_REGISTRATION( MsgQueueTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::MsgQueue;
using CPN::MsgQueueSignal;
using CPN::MsgEmptyChain;
using CPN::MsgChain;
using CPN::MsgMutator;
using CPN::MsgBroadcaster;

class CharMutator {
public:
    CharMutator(char v) : val(v) {}
    char operator()(char c) {
        CPPUNIT_ASSERT(c == val);
        return c + 1;
    }
    char val;
};

void MsgQueueTest::setUp() {
    counter = 0;
}

void MsgQueueTest::tearDown() {
    counter = 0;
}

void MsgQueueTest::Callback() {
    counter++;
}

void MsgQueueTest::TestMsgQueue() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<MsgQueue<char> > msgqueue = MsgQueue<char>::Create();
    msgqueue->Put(1);
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
}

void MsgQueueTest::TestMsgQueueSignal() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<MsgQueueSignal<char, MsgQueue<char> > > msgqueue = MsgQueueSignal<char, MsgQueue<char> >::Create();
    msgqueue->Connect(sigc::mem_fun(this, &MsgQueueTest::Callback));
    msgqueue->Put(1);
    CPPUNIT_ASSERT(1 == counter);
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
}

void MsgQueueTest::TestMsgEmptyChain() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<MsgQueue<char> > msgqueue = MsgQueue<char>::Create();
    shared_ptr<MsgEmptyChain<char> > msgchain = MsgEmptyChain<char>::Create();
    CPPUNIT_ASSERT_THROW(msgchain->Put(1), std::tr1::bad_weak_ptr);
    CPPUNIT_ASSERT(msgchain->Unchained());
    msgchain->Chain(msgqueue);
    CPPUNIT_ASSERT(!msgchain->Unchained());
    msgchain->Put(1);
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
    msgqueue.reset();
    CPPUNIT_ASSERT(msgchain->Unchained());
    CPPUNIT_ASSERT_THROW(msgchain->Put(1), std::tr1::bad_weak_ptr);
}

void MsgQueueTest::TestMsgChain() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<MsgQueue<char> > msgqueue = MsgQueue<char>::Create();
    shared_ptr<MsgChain<char> > msgchain = MsgChain<char>::Create();
    msgchain->Put(1);
    CPPUNIT_ASSERT(msgchain->Unchained());
    msgchain->Chain(msgqueue);
    CPPUNIT_ASSERT(!msgchain->Unchained());
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
    msgchain->Put(1);
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
    msgqueue.reset();
    CPPUNIT_ASSERT(msgchain->Unchained());
}

void MsgQueueTest::TestMsgMutator() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<MsgQueue<char> > msgqueue = MsgQueue<char>::Create();
    shared_ptr<MsgMutator<char, CharMutator> > msgmutate
        = MsgMutator<char, CharMutator>::Create(CharMutator(1));
    CPPUNIT_ASSERT_THROW(msgmutate->Put(1), std::tr1::bad_weak_ptr);
    CPPUNIT_ASSERT(msgmutate->Unchained());
    msgmutate->Chain(msgqueue);
    CPPUNIT_ASSERT(!msgmutate->Unchained());
    msgmutate->Put(1);
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 2);
    CPPUNIT_ASSERT(msgqueue->Empty());
    msgqueue.reset();
    CPPUNIT_ASSERT(msgmutate->Unchained());
    CPPUNIT_ASSERT_THROW(msgmutate->Put(1), std::tr1::bad_weak_ptr);
}

void MsgQueueTest::TestMsgBroadcaster() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<MsgQueue<char> > msgqueue = MsgQueue<char>::Create();
    shared_ptr<MsgQueue<char> > msgqueue1 = MsgQueue<char>::Create();
    shared_ptr<MsgEmptyChain<char> > msgchain = MsgEmptyChain<char>::Create();
    msgchain->Chain(msgqueue1);
    shared_ptr<MsgBroadcaster<char> > msgbroadcast = MsgBroadcaster<char>::Create();
    msgbroadcast->Put(1);
    CPPUNIT_ASSERT(msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue1->Empty());
    msgbroadcast->AddQueue(msgqueue);
    msgbroadcast->AddQueue(msgchain);
    msgbroadcast->Put(1);
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(!msgqueue1->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue1->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue1->Empty());
    msgchain.reset();
    msgbroadcast->Put(1);
    CPPUNIT_ASSERT(msgqueue1->Empty());
    CPPUNIT_ASSERT(!msgqueue->Empty());
    CPPUNIT_ASSERT(msgqueue->Get() == 1);
    CPPUNIT_ASSERT(msgqueue->Empty());
}


