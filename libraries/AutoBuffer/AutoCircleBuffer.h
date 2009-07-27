/** \file
 * A very simple circle buffer (not circular).
 */

#ifndef AUTOCIRCLEBUFFER_H
#define AUTOCIRCLEBUFFER_H

#include "AutoBuffer.h"

class AutoCircleBuffer {
public:
    AutoCircleBuffer() : buff(100), size(0), put(0), get(0) {}
    AutoCircleBuffer(int initialsize) : buff(initialsize), size(0), put(0), get(0) {}
    ~AutoCircleBuffer() {}

    int MaxSize() { return buff.GetSize(); }

    int Size() { return size; }

    char* AllocatePut(int desired, int &actual) {
        actual = desired;
        if (actual > MaxSize() - size) {
            actual = MaxSize() - size;
        }
        if (actual + put > MaxSize()) {
            actual = MaxSize() - put;
        }
        return (char*) buff.GetBuffer(put);
    }
    void ReleasePut(int amount) {
        size += amount;
        put = (put + amount)%MaxSize();
    }

    char* AllocateGet(int desired, int &actual) {
        actual = desired;
        if (actual > size) {
            actual = size;
        }
        if (actual + get > MaxSize()) {
            actual = MaxSize() - get;
        }
        return (char*) buff.GetBuffer(get);
    }
    void ReleaseGet(int amount) {
        size -= amount;
        get = (get + amount)%MaxSize();
    }
    
    void ChangeMaxSize(int newsize) {
        if (newsize < size) {
            newsize = size;
        }
        AutoBuffer storage(size);
        int amount = 0;
        int total = 0;
        while (size > 0) {
            char* ptr = AllocateGet(size, amount);
            storage.Put(ptr, amount, total);
            ReleaseGet(amount);
            total += amount;
        }
        buff.ChangeSize(newsize);
        while (size < total) {
            char* ptr = AllocatePut(total, amount);
            storage.Get(ptr, amount, size);
            ReleasePut(amount);
        }
    }

    void EnsureMaxSize(int newsize) {
        if (MaxSize() < newsize) {
            ChangeMaxSize(newsize);
        }
    }
private:
    AutoBuffer buff;
    int size;
    int put;
    int get;
};

#endif

