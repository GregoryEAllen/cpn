
#pragma once

#include <complex>

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
            unsigned filterlen, short *filt, std::complex<float> *bbcor);
    ~VBeamformer();

    // \return the number of valid output samples
    unsigned Run(std::complex<short> *indata, unsigned instride,
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
