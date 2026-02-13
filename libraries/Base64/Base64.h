//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * Based on the public domain base64 implementation from
 * http://sourceforge.net/projects/libb64
 * \author John Bridgman
 */

#ifndef BASE64_H
#define BASE64_H
#pragma once

#include <string>
#include <vector>

/**
 * \brief Class to handle encoding data into base 64.
 */
class Base64Encoder {
public:
    /**
     * \param cpl characters per line
     * if zero then no line breaks are inserted, otherwise
     * a line break will be inserted every cpl characters.
     */
    Base64Encoder(unsigned cpl = 72);
    /**
     * Encode the data in datain into the buffer passed in codeout.
     * \param datain Data to be encoded
     * \param inlen length of the data to be encoded
     */
    void EncodeBlock(const void *datain, unsigned inlen);

    /**
     * \return Do not end the encoding string but return all that
     * has already been encoded so far.
     */
    std::string GetPartial();
    /**
     * End the encoding and 
     * get the resulting encoded string.
     * \return the encoded string
     */
    std::string BlockEnd();

    /**
     * Reset the encoder to the state it was in after construction.
     */
    void Reset();
private:
    char EncodeValue();
    enum encoderstep {
        step_A, step_B, step_C
    } step;

    const unsigned chars_per_line;
    char result;
    unsigned stepcount;
    std::vector<char> output;
};

/**
 * \brief A class to decode a base 64 string into
 * binary data.
 */
class Base64Decoder {
public:
    Base64Decoder();

    /**
     * Send the string through the decoder.
     * \param code the base 64 encoded data.
     */
    void DecodeBlock(const std::string &code) {
        DecodeBlock(code.data(), code.size());
    }
    /**
     * Send the base 64 encoded data through the decoder.
     * \param codein the code
     * \param codelen the length
     */
    void DecodeBlock(const char *codein, unsigned codelen);
    /**
     * \return the the bytes decoded so far (subsequent calls will not have the bytes).
     */
    std::vector<char> GetPartial();
    /**
     * \return  end the decoding and return the bytes decoded.
     */
    std::vector<char> BlockEnd();
    /**
     * Reset the decoder to the state it was in when first constructed.
     */
    void Reset();
private:
    char DecodeValue(char value_in);
    enum decodestep {
        step_a, step_b, step_c, step_d
    } step;

    char currchar;
    std::vector<char> output;

};

#endif
