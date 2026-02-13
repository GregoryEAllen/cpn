#pragma once
#include <algorithm>

template<typename T>
T Filter(T val, const T *b, unsigned b_len, const T *a, unsigned a_len, T *buf) {
    T out = val * b[0] + buf[0];
    unsigned mlen = std::max(b_len, a_len);
    for (unsigned i = 1; i < mlen; ++i) {
        T v = buf[i];
        if (i < b_len) {
            v += val * b[i];
        }
        if (i < a_len) {
            v -= out * a[i];
        }
        buf[i-1] = v;
    }
    return out;
}
