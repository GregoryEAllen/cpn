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
 * \brief Provide the implementation for the HBLoadFromFile function.
 */
#include "HBeamformer.h"
#include "Variant.h"
#include "Assert.h"
#include "JSONToVariant.h"
#include <complex>
#include <iterator>
#include <fstream>
#include <vector>
using std::complex;

template<typename T>
class VarListIter : public std::iterator<std::input_iterator_tag, T> {
public:
    VarListIter(Variant::ConstListIterator itr_) : itr(itr_) {}
    T operator*() { return itr->AsNumber<T>(); }
    bool operator==(const VarListIter &o) { return o.itr == itr; }
    bool operator!=(const VarListIter &o) { return o.itr != itr; }
    VarListIter operator++() { return VarListIter(++itr); }
    VarListIter operator++(int) { return VarListIter(itr++); }
private:
    Variant::ConstListIterator itr;
};


std::auto_ptr<HBeamformer> HBLoadFromFile(const std::string &filename, bool estimate) {
    std::fstream f;
    f.open(filename.c_str(), std::fstream::in | std::fstream::binary);

    JSONToVariant parser;
    f >> parser;
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    while (f.get() != 0 && f.good());
    ASSERT(f.good());

    unsigned numStaves = header["numStaves"].AsUnsigned();
    unsigned numBeams = header["numBeams"].AsUnsigned();
    unsigned length = header["numSamples"].AsUnsigned();
    std::vector<unsigned> staveIndex(VarListIter<unsigned>(header["staveIndex"].ListBegin()),
            VarListIter<unsigned>(header["staveIndex"].ListEnd()));
    std::vector< complex<float> > coeffs(length * numBeams);
    std::vector< complex<float> > replica(length * numBeams);

    unsigned numread = 0;
    unsigned numtoread = sizeof(complex<float>) * length * numBeams;
    while (f.good() && numread < numtoread) {
        f.read(((char*)&coeffs[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    numread = 0;
    numtoread = sizeof(complex<float>) * length * numBeams;
    while (f.good() && numread < numtoread) {
        f.read(((char*)&replica[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    return std::auto_ptr<HBeamformer>(new HBeamformer(length, numStaves, numBeams, &coeffs[0], &replica[0], &staveIndex[0], estimate));
}

