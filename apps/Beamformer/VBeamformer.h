
#pragma once

#include <complex>

class VBeamformer {
public:

    VBeamformer();
    ~VBeamformer();

    // \return the number of valid output samples
    unsigned Run(std::complex<short> *indata, unsigned instride,
            unsigned numStaves, unsigned numSamples, unsigned fan,
            std::complex<float> *outdata, unsigned outstride);

protected:
    unsigned numFans;
    unsigned numStaveTypes;
    unsigned numElemsPerStave;
    unsigned filterLen;

    float *filter;
    std::complex<float> *bbCorrect;

};

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
