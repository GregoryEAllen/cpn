
#pragma once

#include <complex>
#include <memory>
#include <string>

class FanVBeamformer {
public:
    enum Algorithm_t {
        ALGO_BEGIN = 0,
        GENERIC = 0,
        GENERIC_VECTOR = 1,
        SSE_VECTOR = 2,
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

std::auto_ptr<FanVBeamformer> FanVBLoadFromFile(const std::string &filename);

