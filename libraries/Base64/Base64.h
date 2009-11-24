/** \file
 * Based on the public domain base64 implementation from
 * http://sourceforge.net/projects/libb64
 */

#ifndef BASE64_H
#define BASE64_H
#pragma once

#include <string>
#include <vector>

class Base64Encoder {
public:
    Base64Encoder();
    /**
     * Encode the data in datain into the buffer passed in codeout.
     */
    void EncodeBlock(const void *datain, unsigned inlen);

    std::string GetPartial();
    /**
     * Get the resulting encoded string.
     */
    std::string BlockEnd();

    void Reset();
private:
    char EncodeValue();
    enum encoderstep {
        step_A, step_B, step_C
    } step;

    char result;
    int stepcount;
    std::vector<char> output;
};

class Base64Decoder {
public:
    Base64Decoder();

    void DecodeBlock(const char *codein, unsigned codelen);
    std::vector<char> GetPartial();
    std::vector<char> BlockEnd();
private:
    char DecodeValue(char value_in);
    enum decodestep {
        step_a, step_b, step_c, step_d
    } step;

    char currchar;
    std::vector<char> output;

};

#endif
