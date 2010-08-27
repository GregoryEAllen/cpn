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
#ifndef FANVBEAMFORMER_H
#define FANVBEAMFORMER_H
#pragma once

#include <complex>
#include <memory>
#include <string>

/**
 * A vertical beamformer that produces up to three fans
 * all at once with one pass over the input data.
 */
class FanVBeamformer {
public:
    enum Algorithm_t {
        ALGO_BEGIN = 0,
        AUTO = 0,
        GENERIC = 1,
        GENERIC_VECTOR = 2,
        SSE_VECTOR = 3,
        SSE_VECTOR_HARD = 4,
        SSE_VECTOR_HARDCODE = 5,
        ALGO_END
    };

    FanVBeamformer(unsigned fans, unsigned stavetypes, unsigned elementsperstave,
            unsigned filterlen, float *filt, std::complex<float> *bbcor);

    ~FanVBeamformer();

    struct ResVec {
        ResVec() {}
        ResVec(unsigned f, std::complex<float> *od, unsigned st)
            : fan(f), outdata(od), outstride(st) {}
        unsigned fan;
        std::complex<float> *outdata;
        unsigned outstride;
    };
    unsigned Run(const std::complex<short> *indata, unsigned instride,
            unsigned numStaves, unsigned numSamples, ResVec *rv, unsigned numres);

    unsigned NumFans() const { return numFans; }
    unsigned NumStaveTypes() const { return numStaveTypes; }
    unsigned NumElemsPerStave() const { return numElemsPerStave; }
    unsigned FilterLen() const { return filterLen; }

    Algorithm_t SetAlgorithm(Algorithm_t a) { return algo = a; }
    Algorithm_t GetAlgorithm() const { return algo; }
protected:
    unsigned numFans;
    unsigned numStaveTypes;
    unsigned numElemsPerStave;
    unsigned filterLen;
    float *filter;
    std::complex<float> *bbCorrect;
    Algorithm_t algo;
};

/**
 * Load a FanVBeamformer from a file
 */
std::auto_ptr<FanVBeamformer> FanVBLoadFromFile(const std::string &filename);
#endif
