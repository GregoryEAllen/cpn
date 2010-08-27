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
#ifndef VBEAMFORMER_H
#define VBEAMFORMER_H
#pragma once

#include <complex>
#include <string>
#include <memory>

/**
 * A basic vertical beamformer.
 */
class VBeamformer {
public:
    enum Algorithm_t {
        ALGO_BEGIN = 0,
        GENERIC = 0,
        GENERIC_VECTOR = 1,
        SSE_VECTOR = 2,
        ORIG,
        ALGO_END
    };

    VBeamformer(unsigned fans, unsigned stavetypes, unsigned elementsperstave,
            unsigned filterlen, float *filt, std::complex<float> *bbcor);
    ~VBeamformer();

    /// \return the number of valid output samples
    unsigned Run(const std::complex<short> *indata, unsigned instride,
            unsigned numStaves, unsigned numSamples, unsigned fan,
            std::complex<float> *outdata, unsigned outstride);

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
 * Load the beamformer from the given file.
 */
std::auto_ptr<VBeamformer> VBLoadFromFile(const std::string &filename);
/*
 *
 *  file VBFSteerFilter.pkl
 *
 *  numFans = 3
 *  numStaveTypes = 1
 *  numElemsPerStave = 12
 *  filterLen = 4
 *
 *  payload = short[numFans*numStaveTypes*numElemsPerStave][filterLen]
 *
 *  A file VBFBBCorrect.pkl
 *  payload = complex<float>[numFans*numStaveTypes][numElemsPerStave]
 *
 *  VBFTestInput.pkl
 *
 *  numStaves = 256
 *  numSamples = x
 *  payload = complex<short>[numStaves*numElemsPerStave][numSamples]
 */
#endif
