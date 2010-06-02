
#include "VBeamformer.h"
#include "CircularIterator.h"

#include <complex>

#include <pmmintrin.h>
#include <unistd.h>
#include "ErrnoException.h"

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



VBeamformer::VBeamformer()
{

    AllocAligned(&filter, 16, numFans * numStaveTypes * numElemsPerStave * filterLen);
    AllocAligned(&bbCorrect, 16, numFans * numStaveTypes * numElemsPerStave);
}

VBeamformer::~VBeamformer() {
    free(filter);
    free(bbCorrect);
}

unsigned VBeamformer::Run(std::complex<short> *indata, unsigned instride,
            unsigned numStaves, unsigned numSamples, unsigned fan,
            std::complex<float> *outdata, unsigned outstride)
{
    unsigned outSamples = 0;

    /*
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
            */

    return outSamples;
}


void vbf_generic(
    complex<short> *indata,
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
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        unsigned stavetype = stave % numStaveTypes;
        for (unsigned samp = 0; samp < numOutSamples; ++samp) {
            complex<float> stave_acc = 0;
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                float *filt = &filter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                complex<float> elem_acc = 0;
                for (unsigned fi = 0; fi < filterLen; ++fi) {
                    complex<short> e = instave[elem * instride + samp + fi];
                    elem_acc += complex<float>(e.real(), e.imag()) * filt[fi];
                }
                stave_acc += elem_acc * bbCorrect[stavetype * numElemsPerStave + elem];
            }
            outdata[stave*outstride + samp] = stave_acc;
        }
    }
}

void vbf_generic_vector(
    complex<short> *indata,
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
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        unsigned stavetype = stave % numStaveTypes;
        complex<float> s[numElemsPerStave][BLOCKSIZE + filterLen];
        for (unsigned i = 0; i < numElemsPerStave; ++i) {
            for (unsigned j = BLOCKSIZE; j < BLOCKSIZE + filterLen; ++j) {
                s[i][j] = 0;
            }
        }
        for (unsigned samp = 0; samp < numOutSamples; samp += BLOCKSIZE) {
            complex<float> stave_acc[BLOCKSIZE] = {0};
            for (unsigned elem = 0; elem < numElemsPerStave; ++elem) {
                for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                    complex<short> e = instave[elem * instride + samp + i];
                    s[elem][i] = complex<float>(e.real(), e.imag());
                    s[elem][i] *= bbCorrect[stavetype * numElemsPerStave + elem];
                }
                float *filt = &filter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                for (unsigned fi = 0; fi < filterLen; fi += BLOCKSIZE) {
                    for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                        stave_acc[i] += s[elem][fi + i] * filt[fi];
                    }
                }
                for (unsigned i = filterLen/BLOCKSIZE - 1; i >= 0; --i) {
                    complex<float> *e = &s[elem][i*BLOCKSIZE];
                    for (unsigned j = 0; j < BLOCKSIZE; ++j) {
                        e[j + BLOCKSIZE] = e[j];
                    }
                }
            }
            for (unsigned i = 0; i < BLOCKSIZE; ++i) {
                outdata[stave*outstride + samp + i] = stave_acc[i];
            }
        }
    }
}

void vbf_sse_vector(
    complex<short> *indata,
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
    for (unsigned stave = 0; stave < numStaves; ++stave) {
        complex<short> *instave = &indata[stave * instride * numElemsPerStave];
        unsigned stavetype = stave % numStaveTypes;
        cvector s[numElemsPerStave][filterLen];
        for (unsigned i = 0; i < numElemsPerStave; ++i) {
            for (unsigned j = 0; j < filterLen - CVECSIZE; ++j) {
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
                s[elem][0] = cmult(_mm_cvtepi32_ps(e1), cpx);
				e2 = _mm_unpacklo_epi16(e2, e2);
				e2 = _mm_srai_epi32(e2, 16);
                s[elem][1] = cmult(_mm_cvtepi32_ps(e2), cpx);

                float *filt = &filter[stavetype * numElemsPerStave * filterLen + elem * filterLen];
                for (unsigned fi = 0; fi < BLOCKSPFILT; fi += 1) {
                    vector coeffs = *((vector*)&filt[fi * BLOCKSIZE]);
                    cvector c = _mm_shuffle_ps(coeffs, coeffs, 0x00);
                    stave_acc[0] += s[elem][VECPFILT*fi + 0] * c;
                    stave_acc[1] += s[elem][VECPFILT*fi + 1] * c;
                    c = _mm_shuffle_ps(coeffs, coeffs, 0x55);
                    stave_acc[0] += _mm_shuffle_ps(s[elem][VECPFILT*fi + 0], s[elem][VECPFILT*fi + 1], 0x4E) * c;
                    cvector temp = _mm_shuffle_ps(s[elem][VECPFILT*fi + 1], s[elem][VECPFILT*fi + 2], 0x4E);
                    stave_acc[1] += temp * c;
                    c = _mm_shuffle_ps(coeffs, coeffs, 0xAA);
                    stave_acc[0] += s[elem][VECPFILT*fi + 1] * c;
                    stave_acc[1] += s[elem][VECPFILT*fi + 2] * c;
                    c = _mm_shuffle_ps(coeffs, coeffs, 0xFF);
                    stave_acc[0] += temp * c;
                    stave_acc[1] += _mm_shuffle_ps(s[elem][VECPFILT*fi + 2], s[elem][VECPFILT*fi + 3], 0x4E) * c;
                }
                for (unsigned i = filterLen - BLOCKSIZE - 1; i >= 0; i -= BLOCKSIZE/CVECSIZE) {
                    s[elem][i + 2] = s[elem][i];
                    s[elem][i + 3] = s[elem][i + 1];
                }
            }
            for (unsigned i = 0; i < CVECSIZE; ++i) {
                *((cvector*)&outdata[stave *outstride + samp + i]) = stave_acc[i];
            }
        }
    }
}

#if 0

void vbf_vector(
    complex<short> *indata,
    unsigned numSamples,
    unsigned instride,

    unsigned elemsPerStave,
    unsigned numStaves,

    float *filter,
    unsigned filterLength,
    unsigned numFilters,

    complex<float> *bbCorrect,
    unsigned numBBCorrect, // must be a multiple of numStaves

    complex<float> *outdata,
    unsigned outstride,
    unsigned &numOutSamples
    )
{

	for (int stave = 0; b < numStaves; stave++) {
		__m128 dataa[elemsPerStave], datab[elemsPerStave];

		for (int elem = 0; elem < elemsPerStave; elem++) {
			dataa[elem] = datab[elem] = _mm_setzero_ps();
		}

		for (int samp = 0, samp < numSamples; samp += filterLength) {
            __m128 acc1, acc2;

			acc1 = _mm_setzero_ps();
            acc2 = _mm_setzero_ps();

			for (int elem = 0; elem < elemsPerStave; elem++) {
                __m128 data1, data2;
                __m128 temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
                __m128 mul1, mul2, cpx;

				data1 = (__m128) _mm_load_si128(
                        (__m128i *)&indata[stave * instride * elemsPerStave + elem * instride + samp]
                        );

				// convert to 32-bit floats
				data2 = data1;

				data1 = (__m128) _mm_unpacklo_epi16((__m128i)data1, (__m128i)data1);
				data1 = (__m128) _mm_srai_epi32(data1, 16);
				data1 = _mm_cvtepi32_ps((__m128i) data1);

				data2 = (__m128) _mm_unpackhi_epi16((__m128i)data2, (__m128i)data2);
				data2 = (__m128) _mm_srai_epi32(data2, 16);
				data2 = _mm_cvtepi32_ps((__m128i) data2);

				// complex multiply
				cpx = _mm_load_ps((__m128 *)&bbCorrect[elem]);
                mul1 = ccmult(data1, cpx);
                mul2 = ccmult(data2, cpx);

				// FIR filter
				
                __m128 coeff1, coeff2, coeff3, coeff4;
				coeff4 = (__m128)_mm_load_si128((__m128i *)&filter[0 +
						numFiltReps * kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				coeff3 = (__m128)_mm_load_si128((__m128i *)&filter[1*kBF_NumFilterCoeffs +
						numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				coeff2 = (__m128)_mm_load_si128((__m128i *)&filter[2*kBF_NumFilterCoeffs +
						numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				coeff1 = (__m128)_mm_load_si128((__m128i *)&filter[3*filterLength +
						numFiltReps*filterLength*elem]);

                // alignr_epi8
                // (a:b >> (8* n))
				temp1 = (__m128)_mm_alignr_epi8(datab[k], dataa[k], 8);
                //temp1 = _mm_shuffle_ps(dataa[k], datab[k], 0x4E);
				temp1 = _mm_mul_ps(temp1, coeff1);

				temp2 = (__m128)_mm_alignr_epi8(mul1, datab[k], 8);
                //temp2 = _mm_shuffle_ps(datab[k], mul1, 0x4E);
				temp5 = _mm_mul_ps(temp2, coeff3);
				temp2 = _mm_mul_ps(temp2, coeff1);

				temp3 = _mm_mul_ps(datab[k], coeff2);
				temp4 = _mm_mul_ps(mul1, coeff2);


                temp6 = _mm_shuffle_ps(mul1, mul2, 0x4E);
				//temp6 = (__m128)_mm_alignr_epi8(mul2, mul1, 8);
				temp6 = _mm_mul_ps(temp6, coeff3);

				temp7 = _mm_mul_ps(mul1, coeff4);
				temp8 = _mm_mul_ps(mul2, coeff4);

				dataa[k] = mul1;
                datab[k] = mul2;

				acc1 = _mm_add_ps(acc1, _mm_add_ps(_mm_add_ps(temp1, temp3), _mm_add_ps(temp5, temp7)));
				acc2 = _mm_add_ps(acc2, _mm_add_ps(_mm_add_ps(temp2, temp4), _mm_add_ps(temp6, temp8)));
			}

			vOutData[b*kBF_NumSamples/2 + j/2] = acc1;
			vOutData[b*kBF_NumSamples/2 + j/2 + 1] = acc2;

		}
	}

    numOutSamples = numSamples - filterLength;
}

#endif

#if 0
void VBeamformer::Beamform (short int* inDataPtr, complex<float>* outDataPtr)
{


    for (int fan = 0; fan < kBF_L; fan++) {
        vBeamformKernel_fir(inDataPtr,
                mBF_Coeffs + fan * kBF_NumElemsPerStave * numbbCorrReps * numStaveTypes * 2,
                mFilter + fan * kBF_NumElemsPerStave * numFiltReps * kBF_NumFilterCoeffs * numStaveTypes,
                (__m128 *)(outDataPtr + fan * kBF_NumSamples * kBF_NumStaves));
    }

}


void vBeamformKernel_fir(short* inData, float *cpxcoeffs, float* filter, __m128* vOutData) {

	__m128 zero = { 0, 0, 0, 0 };

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int b = 0; b < kBF_NumStaves; b++) {
		__m128 data1, data2;
		__m128 dataa[kBF_NumElemsPerStave], datab[kBF_NumElemsPerStave], coeff; //data from the previous iteration
		__m128 temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
		__m128 acc1, acc2, mul1, mul2, cpx;
		__m128 coeff1, coeff2, coeff3, coeff4;

		for (int i = 0; i < kBF_NumElemsPerStave; i++) {
			dataa[i] = datab[i] = zero;
		}

		for (int j = 0; j < kBF_NumSamples; j += kBF_NumFilterCoeffs) {

			acc1 = (__m128)_mm_xor_si128((__m128i)acc1, (__m128i)acc1); // zero out these registers/memory locations
			acc2 = (__m128)_mm_xor_si128((__m128i)acc2, (__m128i)acc2); // zero out these registers/memory locations

			for (int k = 0; k < kBF_NumElemsPerStave; k++) {
				data1 = (__m128) _mm_load_si128((__m128i *)&inData[b*kBF_NumSamples*2*kBF_NumElemsPerStave + j*2 + k*kBF_NumSamples*2]);

				// convert to 32-bit floats
				data2 = data1;

				data1 = (__m128) _mm_unpacklo_epi16((__m128i)data1, (__m128i)data1);
				data1 = (__m128) _mm_srai_epi32(data1, 16);
				data1 = _mm_cvtepi32_ps((__m128i) data1);

				data2 = (__m128) _mm_unpackhi_epi16((__m128i)data2, (__m128i)data2);
				data2 = (__m128) _mm_srai_epi32(data2, 16);
				data2 = _mm_cvtepi32_ps((__m128i) data2);

				// complex multiply
				cpx = _mm_load_ps((__m128 *)&cpxcoeffs[k * 2 * numbbCorrReps + (b & 1)* 2 * numbbCorrReps * kBF_NumElemsPerStave]);
				temp3 = _mm_moveldup_ps(data1);
				temp4 = temp2 = cpx;
				temp4 = _mm_mul_ps(temp4, temp3);
				temp3 = _mm_movehdup_ps(data1);
				temp2 = _mm_shuffle_ps(temp2, temp2, 0xb1);
				temp3 = _mm_mul_ps(temp3, temp2);
				mul1 = _mm_addsub_ps(temp4, temp3);

				temp3 = _mm_moveldup_ps(data2);
				temp4 = temp2 = cpx;
				temp4 = _mm_mul_ps(temp4, temp3);
				temp3 = _mm_movehdup_ps(data2);
				temp2 = _mm_shuffle_ps(temp2, temp2, 0xb1);
				temp3 = _mm_mul_ps(temp3, temp2);
				mul2 = _mm_addsub_ps(temp4, temp3);

				// FIR filter
				
				coeff4 = (__m128)_mm_load_si128((__m128i *)&filter[0 +
						numFiltReps * kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				coeff3 = (__m128)_mm_load_si128((__m128i *)&filter[1*kBF_NumFilterCoeffs +
						numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				coeff2 = (__m128)_mm_load_si128((__m128i *)&filter[2*kBF_NumFilterCoeffs +
						numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				coeff1 = (__m128)_mm_load_si128((__m128i *)&filter[3*kBF_NumFilterCoeffs +
						numFiltReps*kBF_NumFilterCoeffs*(k + kBF_NumElemsPerStave*((b & 1)))]);

				temp1 = (__m128)_mm_alignr_epi8(datab[k], dataa[k], 8);
				temp1 = _mm_mul_ps(temp1, coeff1);

				temp2 = (__m128)_mm_alignr_epi8(mul1, datab[k], 8);
				temp2 = _mm_mul_ps(temp2, coeff1);

				temp3 = _mm_mul_ps(datab[k], coeff2);
				temp4 = _mm_mul_ps(mul1, coeff2);

				temp5 = (__m128)_mm_alignr_epi8(mul1, datab[k], 8);
				temp5 = _mm_mul_ps(temp5, coeff3);

				temp6 = (__m128)_mm_alignr_epi8(mul2, mul1, 8);
				temp6 = _mm_mul_ps(temp6, coeff3);

				temp7 = _mm_mul_ps(mul1, coeff4);
				temp8 = _mm_mul_ps(mul2, coeff4);

				dataa[k] = mul1; datab[k] = mul2;

				// debug
				mul1 = _mm_add_ps(_mm_add_ps(temp1, temp3), _mm_add_ps(temp5, temp7));	
				mul2 = _mm_add_ps(_mm_add_ps(temp2, temp4), _mm_add_ps(temp6, temp8));	
				//end debug

				acc1 = _mm_add_ps(acc1, _mm_add_ps(_mm_add_ps(temp1, temp3), _mm_add_ps(temp5, temp7)));
				acc2 = _mm_add_ps(acc2, _mm_add_ps(_mm_add_ps(temp2, temp4), _mm_add_ps(temp6, temp8)));
			}

			vOutData[b*kBF_NumSamples/2 + j/2] = acc1;
			vOutData[b*kBF_NumSamples/2 + j/2 + 1] = acc2;

		}
	}
}
#endif


