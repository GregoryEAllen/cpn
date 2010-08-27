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
 * Basic routines for loading the vertical beamformers from a file.
 */
#include "FanVBeamformer.h"
#include "VBeamformer.h"
#include "Assert.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include <fstream>
#include <vector>

using std::complex;

std::auto_ptr<VBeamformer> VBLoadFromFile(const std::string &filename) {
    std::fstream f;
    f.open(filename.c_str(), std::fstream::in | std::fstream::binary);
    JSONToVariant parser;
    f >> parser;
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    while (f.get() != 0 && f.good());
    ASSERT(f.good());
    unsigned numFans = header["numFans"].AsUnsigned();
    unsigned numStaveTypes = header["numStaveTypes"].AsUnsigned();
    unsigned numElemsPerStave = header["numElemsPerStave"].AsUnsigned();
    unsigned filterLen = header["filterLen"].AsUnsigned();
    std::vector< float > filter(filterLen * numStaveTypes * numElemsPerStave * numFans);
    std::vector< complex<float> > bbcor(numFans * numStaveTypes * numElemsPerStave);
    unsigned numread = 0;
    unsigned numtoread = sizeof(float) * filter.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&filter[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    numread = 0;
    numtoread = sizeof(complex<float>) * bbcor.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&bbcor[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread, "%u != %u", numread, numtoread);
    return std::auto_ptr<VBeamformer>(new VBeamformer(numFans, numStaveTypes, numElemsPerStave, filterLen,
                &filter[0], &bbcor[0]));
}

std::auto_ptr<FanVBeamformer> FanVBLoadFromFile(const std::string &filename) {
    std::fstream f;
    f.open(filename.c_str(), std::fstream::in | std::fstream::binary);
    JSONToVariant parser;
    f >> parser;
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    while (f.get() != 0 && f.good());
    ASSERT(f.good());
    unsigned numFans = header["numFans"].AsUnsigned();
    unsigned numStaveTypes = header["numStaveTypes"].AsUnsigned();
    unsigned numElemsPerStave = header["numElemsPerStave"].AsUnsigned();
    unsigned filterLen = header["filterLen"].AsUnsigned();
    std::vector< float > filter(filterLen * numStaveTypes * numElemsPerStave * numFans);
    std::vector< complex<float> > bbcor(numFans * numStaveTypes * numElemsPerStave);
    unsigned numread = 0;
    unsigned numtoread = sizeof(float) * filter.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&filter[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    numread = 0;
    numtoread = sizeof(complex<float>) * bbcor.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&bbcor[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    return std::auto_ptr<FanVBeamformer>(new FanVBeamformer(numFans, numStaveTypes, numElemsPerStave, filterLen,
                &filter[0], &bbcor[0]));
}


