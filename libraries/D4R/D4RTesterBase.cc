
#include "D4RTesterBase.h"
#include "D4RTestNodeBase.h"
#include "Variant.h"
#include "Assert.h"

namespace D4R {

    TesterBase::TesterBase()
        : success(true)
    {
    }

    TesterBase::~TesterBase() {
    }

    void TesterBase::Setup(const Variant &v) {
        Variant nodes = v["nodes"];
        Variant::ListIterator itr = nodes.ListBegin();
        while (itr != nodes.ListEnd()) {
            CreateNode(*itr);
            ++itr;
        }
        Variant queues = v["queues"];
        itr = queues.ListBegin();
        while (itr != queues.ListEnd()) {
            CreateQueue(*itr);
            ++itr;
        }
    }

    void TesterBase::Failure(TestNodeBase *tnb, const std::string &msg) {
        Error(msg.c_str());
        success = false;
    }
}

