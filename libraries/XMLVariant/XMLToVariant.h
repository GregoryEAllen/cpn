/** \file
 * \author John Bridgman
 * This class provides a way of parsing
 * a simple XML document into a Variant. Note that
 * this operation is only reversable for very simple
 * documents that contain no attributes and whitespace
 * does not matter.
 *
 * Whitespace, commends, and all attributes are stripped.
 * All text inside an element that contains sub elements is
 * ignored. Empty elements are assumed to have the value 'true'.
 *
 * Currently all datatypes are assumed to be strings. The user
 * must then convert them as necessary.
 */
#pragma once
#include "Variant.h"
#include <iosfwd>

class XMLToVariant {
public:
    enum State_t {
        OK,
        DONE,
        ERROR
    };

    XMLToVariant();
    ~XMLToVariant();

    void ParseChunk(const void *ptr, unsigned len);
    void ParseChunk(const std::string &d) { return ParseChunk(d.data(), d.size()); }
    void EndChunk();

    bool ParseFile(const std::string &f);
    bool ParseStream(std::istream &is);

    State_t State() const;
    std::string GetMessage() const;
    Variant Get();
    std::string RootName();

    bool Done() const;
    bool Ok() const;
    bool Error() const;
private:
    class pimpl_t;
    pimpl_t *pimpl;
};

