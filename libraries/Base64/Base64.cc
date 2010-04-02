/** \file
 * Based on the public domain base64 implementation from
 * http://sourceforge.net/projects/libb64
 */

#include "Base64.h"
#include "Assert.h"

static const int CHARS_PER_LINE = 72;

Base64Encoder::Base64Encoder()
    : step(step_A), result(0), stepcount(0)
{
}

char Base64Encoder::EncodeValue()
{
    static const char encoding[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (result > 63) return '=';
    return encoding[(int)result];
}

void Base64Encoder::EncodeBlock(const void *datain, unsigned inlen) {
    const char *current = (const char*)datain;
    const char *const dataend = current + inlen;

    char fragment;
    
    switch (step) {
        while (1) {
    case step_A:
            if (current == dataend) {
                step = step_A;
                return;
            }
            fragment = *current++;
            result = (fragment & 0x0fc) >> 2;
            output.push_back(EncodeValue());
            result = (fragment & 0x003) << 4;
    case step_B:
            if (current == dataend) {
                step = step_B;
                return;
            }
            fragment = *current++;
            result |= (fragment & 0x0f0) >> 4;
            output.push_back(EncodeValue());
            result = (fragment & 0x00f) << 2;
    case step_C:
            if (current == dataend) {
                step = step_C;
                return;
            }
            fragment = *current++;
            result |= (fragment & 0x0c0) >> 6;
            output.push_back(EncodeValue());
            result  = (fragment & 0x03f) >> 0;
            output.push_back(EncodeValue());
            
            ++stepcount;
            if (stepcount == CHARS_PER_LINE/4) {
                output.push_back('\n');
                stepcount = 0;
            }
        }
    }
    ASSERT(false, "control should not reach here");
}

std::string Base64Encoder::GetPartial() {
    std::string ret(&output[0], output.size());
    output.clear();
    return ret;
}

std::string Base64Encoder::BlockEnd() {
    
    switch (step) {
    case step_B:
        output.push_back(EncodeValue());
        output.push_back('=');
        output.push_back('=');
        break;
    case step_C:
        output.push_back(EncodeValue());
        output.push_back('=');
        break;
    case step_A:
        break;
    }
    output.push_back('\n');
    std::string ret(&output[0], output.size());
    Reset();
    return ret;
}

void Base64Encoder::Reset() {
    step = step_A;
    result = 0;
    stepcount = 0;
    output.clear();
}

char Base64Decoder::DecodeValue(char value_in)
{
    static const char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
    static const char decoding_size = sizeof(decoding);
    value_in -= 43;
    if (value_in < 0 || value_in > decoding_size) return -1;
    return decoding[(int)value_in];
}

Base64Decoder::Base64Decoder()
    : step(step_a), currchar(0)
{
}

void Base64Decoder::DecodeBlock(const char *codein, unsigned codelen)
{
    const char* codechar = codein;
    const char *const codeend = codein + codelen;
    char fragment;
    
    switch (step) {
        while (1) {
    case step_a:
            do {
                if (codechar == codeend) {
                    step = step_a;
                    return;
                }
                fragment = DecodeValue(*codechar++);
            } while (fragment < 0);
            currchar = (fragment & 0x03f) << 2;
    case step_b:
            do {
                if (codechar == codeend) {
                    step = step_b;
                    return;
                }
                fragment = DecodeValue(*codechar++);
            } while (fragment < 0);
            currchar |= (fragment & 0x030) >> 4;
            output.push_back(currchar);
            currchar = (fragment & 0x00f) << 4;
    case step_c:
            do {
                if (codechar == codeend) {
                    step = step_c;
                    return;
                }
                fragment = DecodeValue(*codechar++);
            } while (fragment < 0);
            currchar |= (fragment & 0x03c) >> 2;
            output.push_back(currchar);
            currchar = (fragment & 0x003) << 6;
    case step_d:
            do {
                if (codechar == codeend) {
                    step = step_d;
                    return;
                }
                fragment = DecodeValue(*codechar++);
            } while (fragment < 0);
            currchar |= (fragment & 0x03f);
            output.push_back(currchar);
        }
    }
    ASSERT(false, "control should not reach here");
}

std::vector<char> Base64Decoder::GetPartial() {
    std::vector<char> ret;
    ret.swap(output);
    return ret;
}

std::vector<char> Base64Decoder::BlockEnd() {
    std::vector<char> ret;
    ret.swap(output);
    currchar = 0;
    step = step_a;
    return ret;
}

void Base64Decoder::Reset() {
    currchar = 0;
    output.clear();
    step = step_a;
}

