

#include "IntrusiveRing.h"
#include <cstdio>

class Element : public IntrusiveRingElement<Element> {
public:
    Element() : IntrusiveRingElement<Element>(this),
    elem(count++) {}
    int elem;
    static int count;
};

int Element::count = 0;

int main(int argc, char** argv) {
    IntrusiveRing<Element> ring;

    ring.PushFront(new Element);
    ring.PushFront(new Element);
    ring.PushFront(new Element);
    ring.PushFront(new Element);
    ring.PushBack(new Element);

    printf("Forward\n");
    Element *cur = ring.Front();
    do {
        printf("Elem %d\n", cur->elem);
        cur = cur->Next();
    } while (cur != ring.Front());

    printf("In reverse\n");
    cur = ring.Back();
    Element *last = 0;
    do {
        printf("Elem %d\n", cur->elem);
        last = cur;
        cur = cur->Prev();
        //last->Remove();
        delete last;
        last = 0;
    } while (!ring.Empty());

    ring.PushFront(new Element);
    ring.PushFront(new Element);
    ring.PushBack(new Element);
    ring.PushFront(new Element);
    ring.PushBack(new Element);
    ring.PushBack(new Element);

    ring.Recede();

    printf("Forward\n");
    cur = ring.Front();
    do {
        printf("Elem %d\n", cur->elem);
        cur = cur->Next();
    } while (cur != ring.Front());

    printf("Split\n");
    int count = 0;
    while (!ring.Empty()) {
        if ((count % 2) == 0) {
            printf("Front ");
            cur = ring.PopFront();
        } else {
            printf("Back ");
            cur = ring.PopBack();
        }
        printf("Elem %d\n", cur->elem);
        delete cur;
        cur = 0;
        ++count;
    }

    return 0;
}


