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
#ifndef HBEAMFORMER_H
#define HBEAMFORMER_H
#pragma once
#include "fftw_inc.h"
#include <complex>
#include <xmmintrin.h>
#include <vector>
#include <string>
#include <memory>

/**
 * A Basic horizontal beamformer for the fake array.
 */
class HBeamformer {
public:
    HBeamformer(unsigned len, unsigned nStaves, unsigned nBeams,
            const std::complex<float> *coeffs_, const std::complex<float> *replica_,
            const unsigned *staveIndex, bool estimate = false);
    ~HBeamformer();

    unsigned NumVBeams() const { return numVBeams; }
    unsigned NumBeams() const { return numBeams; }
    unsigned NumStaves() const { return numStaves; }
    unsigned NumVStaves() const { return numVStaves; }
    unsigned Length() const { return length; }

    /**
     * run one iteration of the beamformer algorithm
     * \param inptr a pointer to a set a input values
     * \param instride the stride (in input samples) between staves
     * \param outptr a pointer to a buffer to put output data
     * \param outstride the stride (in samples) between output beams.
     */
    void Run(const std::complex<float> *inptr, unsigned instride, std::complex<float> *outptr, unsigned outstride);

    // The output is a block which is length*numVStaves in size
    void RunFirstHalf(const std::complex<float> *inptr, unsigned instride, std::complex<float> *outptr);
    // The input is a block which is length*numVStaves in size
    void RunSecondHalf(const std::complex<float> *inptr, std::complex<float> *outptr, unsigned outstride);
    void PrintTimes();
protected:
    void AllocMem();
    void FreeMem();
    void MakePlans(bool estimate);
    void DestroyPlans();

    void MakeTransposeIndices();

    /// workingData[:,staveToVStaveMap] = fft(inptr).T
    void Stage1(const std::complex<float> *inptr, unsigned instride);
    /// workingData = fft(workingData)
    void Stage2();
    /// workingData *= coeffs
    void Stage3(std::complex<float> *outdata);
    /// workingData = ifft(workingData)
    void Stage4(std::complex<float> *indata);
    /// workingData = workingData.T * replica
    void Stage5();
    /// outptr = ifft(workingData)
    void Stage6(std::complex<float> *outptr, unsigned outstride);

    void TransposeScatter(__m128d *in, __m128d *out);

    const unsigned numVBeams;
    const unsigned numBeams;
    const unsigned numStaves;
    const unsigned numVStaves;
    const unsigned length;


    unsigned *staveToVStaveMap;
    char *transposeIndices;
    std::complex<float>*         workingData;
    std::complex<float>*         scratchData;
    
    std::complex<float>*         coeffs;
    std::complex<float>*         replica;
            
    fftwf_plan              forwardPlan_FFTW_RealGeometry;
    fftwf_plan              inversePlan_FFTW_RealGeometry;

    fftwf_plan              forwardPlan_FFTW_VirtualGeometry;
    fftwf_plan              inversePlan_FFTW_VirtualGeometry;

    std::vector<double> timevals;
};


std::auto_ptr<HBeamformer> HBLoadFromFile(const std::string &filename, bool estimate);

#endif
