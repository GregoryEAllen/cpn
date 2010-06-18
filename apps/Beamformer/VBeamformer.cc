
#include "VBeamformer.h"

#include <complex>

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
const static unsigned FILTER_BLOCK = 4;

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



VBeamformer::VBeamformer(unsigned fans, unsigned stavetypes, unsigned elementsperstave,
        unsigned filterlen, float *filt, complex<float> *bbcor)
    : numFans(fans),
    numStaveTypes(stavetypes),
    numElemsPerStave(elementsperstave),
    filterLen(filterlen),
    algo(VBeamformer::SSE_VECTOR)
{
    AllocAligned(&filter, 16, numFans * numStaveTypes * numElemsPerStave * filterLen);
    AllocAligned(&bbCorrect, 16, numFans * numStaveTypes * numElemsPerStave);
    memcpy(filter, filt, numFans * numStaveTypes * numElemsPerStave * filterLen);
    memcpy(bbCorrect, bbcor, numFans * numStaveTypes * numElemsPerStave * sizeof(complex<float>));
}

VBeamformer::~VBeamformer() {
    free(filter);
    free(bbCorrect);
}

static void vbf_generic(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    complex<float> *outdata,
    unsigned outstride,
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
            complex<float> stave_acc = 0;
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                float *filt = &filter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                complex<short> e = instave[elem * instride + samp];
                s[elem][0] = complex<float>(e.real(), e.imag());
                complex<float> elem_acc = 0;
                for (unsigned fi = 0; fi < filterLen; ++fi) {
                    elem_acc += s[elem][fi] * filt[fi];
                }
                for (unsigned i = filterLen - 1; i > 0; --i) {
                    s[elem][i] = s[elem][i - 1];
                }
                stave_acc += elem_acc * bbCorrect[stavetype * numElemsPerStave + elem];
            }
            outdata[stave*outstride + samp] = stave_acc;
        }
    }
}

static void vbf_generic_vector(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    complex<float> *outdata,
    unsigned outstride,
    unsigned &numOutSamples
    )
{
    /*
     * Assumptions
     * filterLen is a multiple of BLOCKSIZE
     * numSamples is a multiple of BLOCKSIZE
     */
    const unsigned BLOCKSIZE = 4;

    numOutSamples = numSamples - filterLen;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        unsigned stavetype = stave % numStaveTypes;
        const complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        complex<float> s[numElemsPerStave][BLOCKSIZE + filterLen];
        for (unsigned i = 0; i < numElemsPerStave; ++i) {
            for (unsigned j = BLOCKSIZE; j < BLOCKSIZE + filterLen; ++j) {
                s[i][j] = 0;
            }
        }
        for (unsigned samp = 0; samp < numOutSamples; samp += BLOCKSIZE) {
            complex<float> stave_acc[BLOCKSIZE] = {0};
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                float *filt = &filter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                    complex<short> e = instave[elem * instride + samp + i];
                    s[elem][BLOCKSIZE - 1 - i] = complex<float>(e.real(), e.imag());
                    s[elem][BLOCKSIZE - 1 - i] *= bbCorrect[stavetype * numElemsPerStave + elem];
                }
                for (unsigned fi = 0; fi < filterLen; fi += BLOCKSIZE) {
                    for (unsigned j = 0; j < BLOCKSIZE; ++j) {
                        for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                            stave_acc[i] += s[elem][fi + j + i] * filt[fi + j];
                        }
                    }
                }
                for (int i = filterLen; i > 0; i -= BLOCKSIZE) {
                    for (unsigned j = 0; j < BLOCKSIZE; ++j) {
                        s[elem][i + j] = s[elem][i + j - BLOCKSIZE];
                    }
                }
            }
            for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                outdata[stave*outstride + samp + (BLOCKSIZE - 1) - i] = stave_acc[i];
            }
        }
    }
}

static void vbf_sse_vector(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    complex<float> *outdata,
    unsigned outstride,
    unsigned &numOutSamples
    )
{
    /*
     * Assumptions
     * filterLen is a multiple of BLOCKSIZE
     * numSamples is a multiple of BLOCKSIZE
     */
    const unsigned BLOCKSIZE = 4;
    const unsigned CVECSIZE = 2;
    const unsigned BLOCKSPFILT = filterLen/BLOCKSIZE;
    const unsigned VECPFILT = filterLen/CVECSIZE;

    numOutSamples = numSamples - filterLen;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        const complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        unsigned stavetype = stave % numStaveTypes;
        cvector s[numElemsPerStave][VECPFILT + CVECSIZE];
        for (unsigned i = 0; i < numElemsPerStave; ++i) {
            for (unsigned j = 0; j < VECPFILT; ++j) {
                s[i][j + CVECSIZE] = _mm_setzero_ps();
            }
        }
        for (unsigned samp = 0; samp < numOutSamples; samp += BLOCKSIZE) {
            cvector stave_acc[CVECSIZE];
            for (unsigned i = 0; i < CVECSIZE; ++i) {
                stave_acc[i] = _mm_setzero_ps();
            }
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {

                __m128i e1 = *((__m128i*)&instave[elem * instride + samp]);
                __m128i e2 = e1;
                complex<float> bbCor = bbCorrect[stavetype * numElemsPerStave + elem];
                cvector cpx = _mm_setr_ps(bbCor.real(), bbCor.imag(), bbCor.real(), bbCor.imag());
                e1 = _mm_unpacklo_epi16(e1, e1);
                e1 = _mm_srai_epi32(e1, 16);
                s[elem][1] = cmult(_mm_cvtepi32_ps(e1), cpx);
                e2 = _mm_unpackhi_epi16(e2, e2);
                e2 = _mm_srai_epi32(e2, 16);
                s[elem][0] = cmult(_mm_cvtepi32_ps(e2), cpx);

                float *filt = &filter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                for (unsigned fi = 0; fi < BLOCKSPFILT; fi += 1) {
                    vector coeffs = *((vector*)&filt[fi * BLOCKSIZE]);
                    cvector c = _mm_shuffle_ps(coeffs, coeffs, 0x00);
                    stave_acc[0] += s[elem][VECPFILT*fi + 1] * c;
                    stave_acc[1] += s[elem][VECPFILT*fi + 0] * c;
                    c = _mm_shuffle_ps(coeffs, coeffs, 0x55);
                    cvector temp = _mm_shuffle_ps(s[elem][VECPFILT*fi + 2], s[elem][VECPFILT*fi + 1], 0x4E);
                    stave_acc[0] += temp * c;
                    stave_acc[1] += _mm_shuffle_ps(s[elem][VECPFILT*fi + 1], s[elem][VECPFILT*fi + 0], 0x4E) * c;
                    c = _mm_shuffle_ps(coeffs, coeffs, 0xAA);
                    stave_acc[0] += s[elem][VECPFILT*fi + 2] * c;
                    stave_acc[1] += s[elem][VECPFILT*fi + 1] * c;
                    c = _mm_shuffle_ps(coeffs, coeffs, 0xFF);
                    stave_acc[0] += _mm_shuffle_ps(s[elem][VECPFILT*fi + 3], s[elem][VECPFILT*fi + 2], 0x4E) * c;
                    stave_acc[1] += temp * c;
                }
                for (int i = VECPFILT; i > 0; i -= CVECSIZE) {
                    for (unsigned j = 0; j < CVECSIZE; ++j) {
                        s[elem][i + j] = s[elem][i + j - CVECSIZE];
                    }
                }
            }
            for (unsigned i = 0; i < CVECSIZE; ++i) {
                *((cvector*)&outdata[stave *outstride + samp + i*CVECSIZE]) = stave_acc[i];
            }
        }
    }
}

static void vbf_vector(
    const complex<short> *indata,
    unsigned instride,
    unsigned numStaves,
    unsigned numSamples,
    float *filter,
    complex<float> *bbCorrect,
    unsigned numStaveTypes,
    unsigned numElemsPerStave,
    unsigned filterLen,
    complex<float> *outdata,
    unsigned outstride,
    unsigned &numOutSamples
    )
{
    ASSERT(filterLen == 4);
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; stave++) {
        __m128 dataa[numElemsPerStave], datab[numElemsPerStave];
        unsigned stavetype = stave % numStaveTypes;

        for (unsigned elem = 0; elem < numElemsPerStave; elem++) {
            dataa[elem] = datab[elem] = _mm_setzero_ps();
        }

        for (unsigned samp = 0; samp < numSamples; samp += filterLen) {
            __m128 acc1, acc2;

            acc1 = _mm_setzero_ps();
            acc2 = _mm_setzero_ps();

            for (unsigned elem = 0; elem < numElemsPerStave; elem++) {
                __m128 data1, data2;
                __m128 temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
                __m128 mul1, mul2, cpx;

                data1 = (__m128) _mm_load_si128(
                        (__m128i *)&indata[stave * instride * numElemsPerStave + elem * instride + samp]
                        );

                // convert to 32-bit floats
                data2 = data1;

                data1 = (__m128) _mm_unpacklo_epi16((__m128i)data1, (__m128i)data1);
                data1 = (__m128) _mm_srai_epi32((__m128i)data1, 16);
                data1 = _mm_cvtepi32_ps((__m128i) data1);

                data2 = (__m128) _mm_unpackhi_epi16((__m128i)data2, (__m128i)data2);
                data2 = (__m128) _mm_srai_epi32((__m128i)data2, 16);
                data2 = _mm_cvtepi32_ps((__m128i) data2);

                // complex multiply
                complex<float> bbCor = bbCorrect[stavetype * numElemsPerStave + elem];
                cpx = _mm_setr_ps(bbCor.real(), bbCor.imag(), bbCor.real(), bbCor.imag());
                mul1 = cmult(data1, cpx);
                mul2 = cmult(data2, cpx);

                // FIR filter
                
                __m128 coeff1, coeff2, coeff3, coeff4;
                __m128 coeffs = (__m128)_mm_load_si128((__m128i *)&filter[
                        stavetype * numElemsPerStave * filterLen + elem * filterLen]);
                coeff4 = _mm_shuffle_ps(coeffs, coeffs, 0x00);
                coeff3 = _mm_shuffle_ps(coeffs, coeffs, 0x55);
                coeff2 = _mm_shuffle_ps(coeffs, coeffs, 0xAA);
                coeff1 = _mm_shuffle_ps(coeffs, coeffs, 0xFF);
                /*
                coeff4 = (__m128)_mm_load_si128((__m128i *)&filter[0 +
                        numFiltReps * kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

                coeff3 = (__m128)_mm_load_si128((__m128i *)&filter[1*kBF_NumFilterCoeffs +
                        numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

                coeff2 = (__m128)_mm_load_si128((__m128i *)&filter[2*kBF_NumFilterCoeffs +
                        numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

                coeff1 = (__m128)_mm_load_si128((__m128i *)&filter[3*filterLength +
                        numFiltReps*filterLength*elem]);
                        */

                // alignr_epi8
                // (a:b >> (8* n))
                //temp1 = (__m128)_mm_alignr_epi8(datab[elem], dataa[elem], 8);
                temp1 = _mm_shuffle_ps(dataa[elem], datab[elem], 0x4E);
                temp1 = _mm_mul_ps(temp1, coeff1);

                //temp2 = (__m128)_mm_alignr_epi8(mul1, datab[elem], 8);
                temp2 = _mm_shuffle_ps(datab[elem], mul1, 0x4E);
                temp5 = _mm_mul_ps(temp2, coeff3);
                temp2 = _mm_mul_ps(temp2, coeff1);

                temp3 = _mm_mul_ps(datab[elem], coeff2);
                temp4 = _mm_mul_ps(mul1, coeff2);


                //temp6 = (__m128)_mm_alignr_epi8(mul2, mul1, 8);
                temp6 = _mm_shuffle_ps(mul1, mul2, 0x4E);
                temp6 = _mm_mul_ps(temp6, coeff3);

                temp7 = _mm_mul_ps(mul1, coeff4);
                temp8 = _mm_mul_ps(mul2, coeff4);

                dataa[elem] = mul1;
                datab[elem] = mul2;

                acc1 = _mm_add_ps(acc1, _mm_add_ps(_mm_add_ps(temp1, temp3), _mm_add_ps(temp5, temp7)));
                acc2 = _mm_add_ps(acc2, _mm_add_ps(_mm_add_ps(temp2, temp4), _mm_add_ps(temp6, temp8)));
            }

            *((__m128*)&outdata[stave * outstride + samp + 0]) = acc1;
            *((__m128*)&outdata[stave * outstride + samp + 2]) = acc2;
        }
    }

    numOutSamples = numSamples - filterLen;
}


unsigned VBeamformer::Run(const std::complex<short> *indata, unsigned instride,
            unsigned numStaves, unsigned numSamples, unsigned fan,
            std::complex<float> *outdata, unsigned outstride)
{
    unsigned outSamples = 0;

    switch (algo) {
    case GENERIC:
        vbf_generic(
                indata,
                instride,
                numStaves,
                numSamples,
                &filter[fan * numStaveTypes * numElemsPerStave * filterLen],
                &bbCorrect[fan * numStaveTypes * numElemsPerStave],
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                outdata,
                outstride,
                outSamples
                );
        break;
    case GENERIC_VECTOR:
        vbf_generic_vector(
                indata,
                instride,
                numStaves,
                numSamples,
                &filter[fan * numStaveTypes * numElemsPerStave * filterLen],
                &bbCorrect[fan * numStaveTypes * numElemsPerStave],
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                outdata,
                outstride,
                outSamples
                );

        break;
    case SSE_VECTOR:
        vbf_sse_vector(
                indata,
                instride,
                numStaves,
                numSamples,
                &filter[fan * numStaveTypes * numElemsPerStave * filterLen],
                &bbCorrect[fan * numStaveTypes * numElemsPerStave],
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                outdata,
                outstride,
                outSamples
                );

        break;
    case ORIG:
        vbf_vector(
                indata,
                instride,
                numStaves,
                numSamples,
                &filter[fan * numStaveTypes * numElemsPerStave * filterLen],
                &bbCorrect[fan * numStaveTypes * numElemsPerStave],
                numStaveTypes,
                numElemsPerStave,
                filterLen,
                outdata,
                outstride,
                outSamples
                );
    default:
        break;
    }
    return outSamples;
}


