
#include "FanVBeamformer.h"
#include <tmmintrin.h>
#include <unistd.h>
#include <string.h>
#include "ErrnoException.h"
#include "Assert.h"

using std::complex;

template<typename T>
T *AllocAligned(T **ptr, unsigned alignment, unsigned size) {
    void *ret = 0;
    int err = posix_memalign(&ret, alignment, size * sizeof(T));
    if (err) throw ErrnoException(err);
    //ret = memalign(alignment, size * sizeof(T));
    //if (!ret) throw ErrnoException();
    if (ptr) *ptr = (T*)ret;
    return (T*)ret;
}

typedef __m128 vector; // float[4]
typedef __m128 cvector;
// complex<float>[2]

static inline cvector cmult(cvector a, cvector b) {
    /*
    cvector c, d;
    c = _mm_moveldup_ps(a);
    d = _mm_mul_ps(c, b);
    c = _mm_shuffle_ps(b, b, 0xb1);
    a = _mm_movehdup_ps(a);
    a = _mm_mul_ps(a, c);
    return _mm_addsub_ps(d, a);
    */
    return _mm_addsub_ps(_mm_mul_ps(_mm_moveldup_ps(a), b),
            _mm_mul_ps(_mm_movehdup_ps(a), _mm_shuffle_ps(b, b, 0xb1)));
}

FanVBeamformer::FanVBeamformer(unsigned fans, unsigned stavetypes, unsigned elementsperstave,
            unsigned filterlen, float *filt, std::complex<float> *bbcor)
    : numFans(fans),
    numStaveTypes(stavetypes),
    numElemsPerStave(elementsperstave),
    filterLen(filterlen)
{
    AllocAligned(&filter, 16, numFans * numStaveTypes * numElemsPerStave * filterLen);
    AllocAligned(&bbCorrect, 16, numFans * numStaveTypes * numElemsPerStave);
    memcpy(filter, filt, numFans * numStaveTypes * numElemsPerStave * filterLen * sizeof(float));
    memcpy(bbCorrect, bbcor, numFans * numStaveTypes * numElemsPerStave * sizeof(complex<float>));
}

FanVBeamformer::~FanVBeamformer() {
    free(filter);
    free(bbCorrect);
}

static void fvbf_generic(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    FanVBeamformer::ResVec *rv,
    unsigned numres,
    unsigned &numOutSamples
    )
{
    numOutSamples = numSamples - filterLen;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        unsigned stavetype = stave % numStaveTypes;
        const complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        complex<float> s[numElemsPerStave][filterLen];
        for (unsigned j = 0; j < numElemsPerStave; ++j) {
            for (unsigned i = 0; i < filterLen; ++i) {
                s[j][i] = 0;
            }
        }
        for (unsigned samp = 0; samp < numOutSamples; ++samp) {
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                complex<short> e = instave[elem * instride + samp];
                s[elem][0] = complex<float>(e.real(), e.imag());
            }
            for (unsigned res = 0; res < numres; ++res) {

                unsigned fan = rv[res].fan;
                complex<float> *outdata = rv[res].outdata;
                unsigned outstride = rv[res].outstride;
                float *fanfilter = &filter[fan * numStaveTypes * numElemsPerStave * filterLen];
                complex<float> *fanbbCorrect = &bbCorrect[fan * numStaveTypes * numElemsPerStave];

                complex<float> stave_acc = 0;
                for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                    float *filt = &fanfilter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                    complex<float> elem_acc = 0;
                    for (unsigned fi = 0; fi < filterLen; ++fi) {
                        elem_acc += s[elem][fi] * filt[fi];
                    }
                    stave_acc += elem_acc * fanbbCorrect[stavetype * numElemsPerStave + elem];
                }
                outdata[stave*outstride + samp] = stave_acc;
            }
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                for (unsigned i = filterLen - 1; i > 0; --i) {
                    s[elem][i] = s[elem][i - 1];
                }
            }
        }
    }
}

static void fvbf_generic_vector(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    FanVBeamformer::ResVec *rv,
    unsigned numres,
    unsigned &numOutSamples
    )
{

    const unsigned BLOCKSIZE = 4;
    numOutSamples = numSamples - filterLen;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        unsigned stavetype = stave % numStaveTypes;
        const complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        complex<float> s[numElemsPerStave][BLOCKSIZE + filterLen];
        for (unsigned j = 0; j < numElemsPerStave; ++j) {
            for (unsigned i = BLOCKSIZE; i < BLOCKSIZE + filterLen; ++i) {
                s[j][i] = 0;
            }
        }
        for (unsigned samp = 0; samp < numOutSamples; samp += BLOCKSIZE) {
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                    complex<short> e = instave[elem * instride + samp + i];
                    s[elem][BLOCKSIZE - 1 - i] = complex<float>(e.real(), e.imag());
                }
            }
            for (unsigned res = 0; res < numres; ++res) {

                unsigned fan = rv[res].fan;
                complex<float> *outdata = rv[res].outdata;
                unsigned outstride = rv[res].outstride;
                float *fanfilter = &filter[fan * numStaveTypes * numElemsPerStave * filterLen];
                complex<float> *fanbbCorrect = &bbCorrect[fan * numStaveTypes * numElemsPerStave];

                complex<float> stave_acc[BLOCKSIZE] = {0};
                for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                    float *filt = &fanfilter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                    complex<float> elem_acc[BLOCKSIZE] = {0};

                    for (unsigned fi = 0; fi < filterLen; fi += BLOCKSIZE) {
                        for (unsigned j = 0; j < BLOCKSIZE; ++j) {
                            for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                                elem_acc[i] += s[elem][fi + j + i] * filt[fi + j];
                            }
                        }
                    }
                    for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                        stave_acc[i] += elem_acc[i] * fanbbCorrect[stavetype * numElemsPerStave + elem];
                    }
                }
                for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                    outdata[stave*outstride + samp + (BLOCKSIZE - 1) - i] = stave_acc[i];
                }
            }
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                for (int i = filterLen; i > 0; i -= BLOCKSIZE) {
                    for (unsigned j = 0; j < BLOCKSIZE; ++j) {
                        s[elem][i + j] = s[elem][i + j - BLOCKSIZE];
                    }
                }
            }
        }
    }
}

static void fvbf_sse_vector(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    FanVBeamformer::ResVec *rv,
    unsigned numres,
    unsigned &numOutSamples
    )
{

    const unsigned BLOCKSIZE = 4;
    const unsigned CVECSIZE = 2;
    const unsigned BLOCKSPFILT = filterLen/BLOCKSIZE;
    const unsigned VECPFILT = filterLen/CVECSIZE;

    numOutSamples = numSamples - filterLen;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        unsigned stavetype = stave % numStaveTypes;
        const complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        cvector s[numElemsPerStave][VECPFILT + CVECSIZE];
        for (unsigned i = 0; i < numElemsPerStave; ++i) {
            for (unsigned j = 0; j < VECPFILT; ++j) {
                s[i][j + CVECSIZE] = _mm_setzero_ps();
            }
        }
        for (unsigned samp = 0; samp < numOutSamples; samp += BLOCKSIZE) {
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                __m128i e1 = *((__m128i*)&instave[elem * instride + samp]);
                __m128i e2 = e1;
                e1 = _mm_unpacklo_epi16(e1, e1);
                e1 = _mm_srai_epi32(e1, 16);
                s[elem][1] = _mm_cvtepi32_ps(e1);
                e2 = _mm_unpackhi_epi16(e2, e2);
                e2 = _mm_srai_epi32(e2, 16);
                s[elem][0] = _mm_cvtepi32_ps(e2);
            }
            for (unsigned res = 0; res < numres; ++res) {

                unsigned fan = rv[res].fan;
                complex<float> *outdata = rv[res].outdata;
                unsigned outstride = rv[res].outstride;
                float *fanfilter = &filter[fan * numStaveTypes * numElemsPerStave * filterLen + stavetype * numElemsPerStave * filterLen];
                complex<float> *fanbbCorrect = &bbCorrect[fan * numStaveTypes * numElemsPerStave];

                cvector stave_acc[CVECSIZE];
                for (unsigned i = 0; i < CVECSIZE; ++i) {
                    stave_acc[i] = _mm_setzero_ps();
                }
                for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                    float *filt = &fanfilter[elem * filterLen];
                    cvector elem_acc[CVECSIZE];
                    for (unsigned i = 0; i < CVECSIZE; ++i) {
                        elem_acc[i] = _mm_setzero_ps();
                    }

                    for (unsigned fi = 0; fi < BLOCKSPFILT; fi += 1) {
                        vector coeffs = *((vector*)&filt[fi * BLOCKSIZE]);
                        cvector c = _mm_shuffle_ps(coeffs, coeffs, 0x00);
                        elem_acc[0] += s[elem][VECPFILT*fi + 1] * c;
                        elem_acc[1] += s[elem][VECPFILT*fi + 0] * c;
                        c = _mm_shuffle_ps(coeffs, coeffs, 0x55);
                        cvector temp = _mm_shuffle_ps(s[elem][VECPFILT*fi + 2], s[elem][VECPFILT*fi + 1], 0x4E);
                        elem_acc[0] += temp * c;
                        elem_acc[1] += _mm_shuffle_ps(s[elem][VECPFILT*fi + 1], s[elem][VECPFILT*fi + 0], 0x4E) * c;
                        c = _mm_shuffle_ps(coeffs, coeffs, 0xAA);
                        elem_acc[0] += s[elem][VECPFILT*fi + 2] * c;
                        elem_acc[1] += s[elem][VECPFILT*fi + 1] * c;
                        c = _mm_shuffle_ps(coeffs, coeffs, 0xFF);
                        elem_acc[0] += _mm_shuffle_ps(s[elem][VECPFILT*fi + 3], s[elem][VECPFILT*fi + 2], 0x4E) * c;
                        elem_acc[1] += temp * c;
                    }
                    complex<float> bbCor = fanbbCorrect[stavetype * numElemsPerStave + elem];
                    cvector cpx = _mm_setr_ps(bbCor.real(), bbCor.imag(), bbCor.real(), bbCor.imag());
                    for (unsigned i = 0; i < CVECSIZE; ++i) {
                        stave_acc[i] += cmult(elem_acc[i], cpx);
                    }
                }
                for (unsigned i = 0; i < CVECSIZE; ++i) {
                    *((cvector*)&outdata[stave *outstride + samp + i*CVECSIZE]) = stave_acc[i];
                }
            }
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                for (int i = VECPFILT; i > 0; i -= CVECSIZE) {
                    for (unsigned j = 0; j < CVECSIZE; ++j) {
                        s[elem][i + j] = s[elem][i + j - CVECSIZE];
                    }
                }
            }
        }
    }
}

unsigned FanVBeamformer::Run(const std::complex<short> *indata, unsigned instride,
        unsigned numStaves, unsigned numSamples, ResVec *rv, unsigned numres) {
    unsigned numOutputSamples = 0;;
    switch (algo) {
    case GENERIC:
        fvbf_generic(
                indata,
                instride,
                numStaves,
                numSamples,
                filter,
                bbCorrect,
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                rv,
                numres,
                numOutputSamples
                );
        break;
    case GENERIC_VECTOR:
        fvbf_generic_vector(
                indata,
                instride,
                numStaves,
                numSamples,
                filter,
                bbCorrect,
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                rv,
                numres,
                numOutputSamples
                );
        break;
    case SSE_VECTOR:
        fvbf_sse_vector(
                indata,
                instride,
                numStaves,
                numSamples,
                filter,
                bbCorrect,
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                rv,
                numres,
                numOutputSamples
                );
        break;
    default:
        break;
    }
    return numOutputSamples;
}

