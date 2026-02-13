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
 * \author John Bridgman
 */
#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP
#pragma once
#include "JSON_parser.h"
#include <string>
#include <stdint.h>
#include <iosfwd>
namespace JSON {
    /**
     * A class that encapsulates the JSON_parser with some extra
     * methods and convienence functions.
     */
    class Parser {
    public:
        enum Status_t {
            OK,
            ERROR,
            DONE
        };
        Parser();
        virtual ~Parser();

        /**
         * Parse one character.
         * \param c the character
         * \return true if everything is good, false if something is
         * odd check the status.
         */
        bool Parse(int c);

        /**
         * Parse an array.
         * \return number of characters accepted by the parser.
         */
        unsigned Parse(const char *c, unsigned len);
        /**
         * Parse a std::string.
         * \return number of characters accepted by the parser.
         */
        unsigned Parse(const std::string &str) { return Parse(str.data(), str.size()); }

        /**
         * Read characters from the std::istream until
         * some error occures (check status of parser and istream)
         */
        void ParseStream(std::istream &is);
        /**
         * Read characters from a standard FILE
         */
        void ParseFile(FILE *f);
        /**
         * Open the given file and read from it.
         */
        bool ParseFile(const std::string &filename);

        /**
         * \return the status of the parser.
         */
        Status_t GetStatus() const { return status; }
        bool Done() const { return status == DONE; }
        bool Error() const { return status == ERROR; }
        bool Ok() const { return status == OK; }
        unsigned GetLine() const { return line; }
        unsigned GetColumn() const { return column; }
        unsigned GetByteCount() const { return charcount; }
        /**
         * Resets the parser to the state it was in when just constructed.
         */
        virtual void Reset();
    protected:

        /**
         * Called when an array begins.
         * \return true on success, false if an error occured.
         */
        virtual bool ArrayBegin() = 0;
        /**
         * Called when an array ends.
         * \return true on success, false if an error occured.
         */
        virtual bool ArrayEnd() = 0;
        /**
         * Called when an object begins.
         * \return true on success, false if an error occured.
         */
        virtual bool ObjectBegin() = 0;
        /**
         * Called when an object end.
         * \return true on success, false if an error occured.
         */
        virtual bool ObjectEnd() = 0;
        /**
         * Called when an integer is encountered.
         * \param value the value of the integer
         * \return true on success, false if an error occured.
         */
        virtual bool Integer(int64_t value) = 0;
        /**
         * Called when an float is encountered.
         * \param value the value of the float
         * \return true on success, false if an error occured.
         */
        virtual bool Float(double value) = 0;
        /**
         * Called when an string is encountered.
         * \param str the value of the string
         * \return true on success, false if an error occured.
         */
        virtual bool String(const std::string &str) = 0;
        /**
         * Called when an null is encountered.
         * \return true on success, false if an error occured.
         */
        virtual bool Null() = 0;
        /**
         * Called when an true is encountered.
         * \return true on success, false if an error occured.
         */
        virtual bool True() = 0;
        /**
         * Called when an false is encountered.
         * \return true on success, false if an error occured.
         */
        virtual bool False() = 0;
        /**
         * Called when an key is encountered.
         * \param str the value of the key
         * \return true on success, false if an error occured.
         */
        virtual bool Key(const std::string &str) = 0;

        static int StaticCallback(void *ctx, int type, const struct JSON_value_struct* value);
        int Callback(int type, const struct JSON_value_struct* value);

        JSON_parser parser;
        Status_t status;

        unsigned line;
        unsigned column;
        unsigned charcount;
        unsigned depth;
    private:
        Parser(const Parser  &);
        Parser & operator=(const Parser &);
        void AllocParser();
    };
    std::istream &operator>>(std::istream &is, Parser &p);
}


#endif
