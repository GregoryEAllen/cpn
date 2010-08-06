
#include "HBeamformer.h"
#include "Assert.h"
#include "ErrnoException.h"
#include "NumProcs.h"
#include <pmmintrin.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef DUMPING
#include <sstream>
#include "LoadFromFile.h"
#endif

// Maybe adjust the loop blocking based on this CACHE_LENGTH rather than set it explicitly?
#define CACHE_LENGTH 64

using std::complex;

void transpose_and_cmul_matrix(__m128d *in, __m128d *out, __m128d *mul, int M, int N);
void vec_cpx_mul(const float* a, const float* b, float* d, int count);
void transpose_matrix(__m128d *in, __m128d *out, int M, int N, int blockM, int blockN);

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

HBeamformer::HBeamformer(unsigned len, unsigned nStaves, unsigned nBeams,
            const std::complex<float> *coeffs_, const std::complex<float> *replica_,
            const unsigned *staveIndex, bool estimate)
    : numVBeams(nBeams),
    numBeams(nBeams),
    numStaves(nStaves),
    numVStaves(nBeams),
    length(len),
    staveToVStaveMap(0),
    transposeIndices(0),
    workingData(0),
    scratchData(0),
    coeffs(0),
    replica(0)
{
    try {
        AllocMem();
    } catch (...) {
        FreeMem();
        throw;
    }
    transpose_matrix((__m128d*)coeffs_, (__m128d*)coeffs, numVBeams, length,
            numVBeams/(CACHE_LENGTH/sizeof(__m128d)), (CACHE_LENGTH/sizeof(__m128d)));
    for (unsigned i = 0; i < numVBeams * length; ++i) {
        coeffs[i] /= (float)(numVBeams * length);
    }
    memcpy(replica, replica_, sizeof(complex<float>) * numVBeams * length);
    memcpy(staveToVStaveMap, staveIndex, sizeof(unsigned) * numStaves);
    MakeTransposeIndices();
    MakePlans(estimate);
}

HBeamformer::~HBeamformer() {
    DestroyPlans();
    FreeMem();
}

void HBeamformer::MakeTransposeIndices() {
    unsigned transval = 0;
    unsigned i = 0;
    for (unsigned stave = 0; stave < numStaves; stave++) {
        while (true) {
            transval = ((transval << 1) | ((i == staveToVStaveMap[stave]) ? 1 : 0)) & 0x3;
            if (i & 0x1) {
                transposeIndices[i >> 1] = transval;
            }
            ++i;
            if (transval & 0x1) break;
        }
    }
    while (i < numVStaves) {
        transval = (transval << 1) & 0x3;
        if (i & 0x1) {
            transposeIndices[i >> 1] = transval;
        }
        ++i;
    }
}

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

void HBeamformer::AllocMem() {
    AllocAligned(&coeffs, 16, length * numVBeams);
    AllocAligned(&replica, 16, length * numVBeams);

    staveToVStaveMap = (unsigned*)malloc(sizeof(unsigned)*numStaves);
    if (!staveToVStaveMap) throw ErrnoException();
    transposeIndices = (char*)malloc(sizeof(char)*numVStaves/2);
    if (!transposeIndices) throw ErrnoException();

    AllocAligned(&workingData, 16, length * numVStaves);
    AllocAligned(&scratchData, 16, length * numVBeams);
}

void HBeamformer::FreeMem() {
    free(coeffs);
    free(replica);

    free(staveToVStaveMap);
    free(transposeIndices);

    free(workingData);
    free(scratchData);
}

void HBeamformer::MakePlans(bool estimate) {
    PthreadMutexProtected pmp(fftw_lock);
    fftwf_init_threads();

    int fftwFlags = FFTW_MEASURE;
    if (estimate)
        fftwFlags = FFTW_ESTIMATE;
    
    forwardPlan_FFTW_RealGeometry = ::fftwf_plan_dft_1d (length, 
                                                (fftwf_complex*) scratchData, 
                                                (fftwf_complex*) workingData, 
                                                FFTW_FORWARD, 
                                                fftwFlags);

#ifdef _OPENMP
    fftwf_plan_with_nthreads(GetNumProcs());
#endif
    const int vstaves = numVStaves;
    forwardPlan_FFTW_VirtualGeometry = ::fftwf_plan_many_dft(1, &vstaves, length,
                                                (fftwf_complex*) scratchData, NULL,
                                                1, numVStaves,
                                                (fftwf_complex*) workingData, NULL,
                                                1, numVStaves,
                                                FFTW_FORWARD, 
                                                fftwFlags | FFTW_DESTROY_INPUT);


    const int vbeams = numVBeams;
    inversePlan_FFTW_VirtualGeometry = ::fftwf_plan_many_dft(1, &vbeams, length,
                                                (fftwf_complex*) scratchData, NULL,
                                                1, numVBeams,
                                                (fftwf_complex*) scratchData, NULL,
                                                1, numVBeams,
                                                FFTW_BACKWARD, 
                                                fftwFlags | FFTW_DESTROY_INPUT);

#ifdef _OPENMP
    fftwf_plan_with_nthreads(1);
#endif
    inversePlan_FFTW_RealGeometry = ::fftwf_plan_dft_1d (length, 
                                                (fftwf_complex*) scratchData, 
                                                (fftwf_complex*) workingData, 
                                                FFTW_BACKWARD, 
                                                fftwFlags | FFTW_DESTROY_INPUT);
}

void HBeamformer::DestroyPlans() {
    PthreadMutexProtected pmp(fftw_lock);
    fftwf_destroy_plan(forwardPlan_FFTW_RealGeometry);
    fftwf_destroy_plan(forwardPlan_FFTW_VirtualGeometry);
    fftwf_destroy_plan(inversePlan_FFTW_VirtualGeometry);
    fftwf_destroy_plan(inversePlan_FFTW_RealGeometry);
}

#ifdef DUMPING
void Dump(const complex<float> *p, unsigned length, unsigned numchans, unsigned chanstride) {
    static int num = 0;
    std::ostringstream oss;
    oss << "dump." << num;
    FILE *f = fopen(oss.str().c_str(), "w");
    DataToFile(f, p, sizeof(complex<float>)*length, sizeof(complex<float>)*chanstride, numchans);
    fclose(f);
    ++num;
}
#endif

void HBeamformer::Run(const complex<float> *inptr, unsigned instride,
        complex<float> *outptr, unsigned outstride) {
    timevals.clear();
    timevals.push_back(getTime());
#ifdef DUMPING
    Dump(inptr, length, numStaves, instride);
#endif
    Stage1(inptr, instride);
#ifdef DUMPING
    Dump(workingData, numVStaves, length, numVStaves);
#endif
    timevals.push_back(getTime());
    Stage2();
#ifdef DUMPING
    Dump(workingData, numVStaves, length, numVStaves);
#endif
    timevals.push_back(getTime());
    Stage3(scratchData);
    std::swap(workingData, scratchData);
#ifdef DUMPING
    Dump(workingData, numVBeams, length, numVBeams);
#endif
    timevals.push_back(getTime());
    Stage4(workingData);
#ifdef DUMPING
    Dump(workingData, numVBeams, length, numVBeams);
#endif
    timevals.push_back(getTime());
    Stage5();
    timevals.push_back(getTime());
#ifdef DUMPING
    Dump(workingData, length, numBeams, length);
#endif
    Stage6(outptr, outstride);
#ifdef DUMPING
    Dump(outptr, length, numBeams, outstride);
#endif
    timevals.push_back(getTime());
}

void HBeamformer::RunFirstHalf(const std::complex<float> *inptr, unsigned instride,
        std::complex<float> *outptr) {
    timevals.clear();
    timevals.push_back(getTime());
    Stage1(inptr, instride);
    timevals.push_back(getTime());
    Stage2();
    timevals.push_back(getTime());
    Stage3(outptr);
    timevals.push_back(getTime());
}

void HBeamformer::RunSecondHalf(const std::complex<float> *inptr,
        std::complex<float> *outptr, unsigned outstride) {
    timevals.clear();
    timevals.push_back(getTime());
    Stage4(const_cast<complex<float>*>(inptr));
    timevals.push_back(getTime());
    Stage5();
    timevals.push_back(getTime());
    Stage6(outptr, outstride);
    timevals.push_back(getTime());
}

void HBeamformer::Stage1(const complex<float> *inptr, unsigned instride) {
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned stave = 0; stave < numStaves; stave++) {
        fftwf_complex *in = (fftwf_complex*)  const_cast<complex<float>*> (inptr) 
            + stave * instride;
        fftwf_complex *out = (fftwf_complex*)  workingData 
            + staveToVStaveMap[stave] * length;

        ::fftwf_execute_dft (forwardPlan_FFTW_RealGeometry, in, out);
    }

    timevals.push_back(getTime());
    TransposeScatter((__m128d*) workingData,
                        (__m128d*) scratchData);
    std::swap(workingData, scratchData);
}

void HBeamformer::Stage2() {
    ::fftwf_execute_dft (forwardPlan_FFTW_VirtualGeometry, 
                            (fftwf_complex*)  workingData, 
                            (fftwf_complex*)  scratchData);
    std::swap(workingData, scratchData);
}

void HBeamformer::Stage3(std::complex<float> *outdata) {
    vec_cpx_mul(
            (float*) workingData,
            (float*) coeffs,
            (float*) outdata,
            numVStaves * length);
}

void HBeamformer::Stage4(std::complex<float> *indata) {
    ::fftwf_execute_dft (inversePlan_FFTW_VirtualGeometry, 
                            (fftwf_complex*)  indata, 
                            (fftwf_complex*)  workingData);

    /*
    for (unsigned i = 0; i < numVBeams*length; ++i) {
        workingData[i] /= numVBeams;
    }
    */
}

void HBeamformer::Stage5() {
    transpose_and_cmul_matrix((__m128d *) workingData, 
                        (__m128d *) scratchData, 
                        (__m128d *) replica,
                        length, numBeams);
    std::swap(workingData, scratchData);
}

void HBeamformer::Stage6(complex<float> *outptr, unsigned outstride) {
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned beam = 0; beam < numBeams; beam++) {
        fftwf_complex *in = (fftwf_complex*) (workingData + (beam * length));
        fftwf_complex *out = (fftwf_complex*) (outptr + (beam * outstride));
        ::fftwf_execute_dft (inversePlan_FFTW_RealGeometry, in, out);
        /*
        for (unsigned i = 0; i < length; ++i) {
            outptr[i + (beam * outstride)] /= length;
        }
        */
    }
}

void HBeamformer::PrintTimes() {
    FILE *f = stderr;

    int i = 0;
    while (i + 1 < timevals.size()) {
        double thetime = timevals[1 + i] - timevals[i];
        fprintf(f, "Stage%d:\t%f\n", i, thetime);
        ++i;
    }
    fprintf(f, "total:\t%f\n", timevals[i] - timevals[0]);
}


void HBeamformer::TransposeScatter(__m128d *in, __m128d *out) {

    const unsigned M = numVStaves;
    const unsigned N = length; 
    const unsigned blockM = M/4;
    const unsigned blockN = 4;

    __m128d zero_vector = {0,0};

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (unsigned i = 0; i < M/2; i += blockM/2) {
        for (unsigned j = 0; j < N/2; j += blockN/2) {
            for (unsigned ii = i; ii < i + blockM/2; ii+=1) {
                switch (transposeIndices[ii]) {
                    case 0:
                        for (unsigned jj = j; jj < j + blockN/2; jj+=1 ) {
                            out[ii + jj*M] = zero_vector;
                            out[ii + (jj*M)+M/2] = zero_vector;
                        }
                        break;
                    case 1:
                        for (unsigned jj = j; jj < j + blockN/2; jj+=1 ) {
                            __m128d y = in[jj + (ii*N) + N/2];
                            out[ii + jj*M] =  _mm_unpacklo_pd(zero_vector, y);
                            out[ii + (jj*M)+M/2] = _mm_unpackhi_pd(zero_vector, y);
                        }
                        break;
                    case 2:
                        for (unsigned jj = j; jj < j + blockN/2; jj+=1 ) {
                            __m128d x = in[jj + ii*N];
                            out[ii + jj*M] = _mm_unpacklo_pd(x, zero_vector);
                            out[ii + (jj*M)+M/2] = _mm_unpackhi_pd(x, zero_vector);
                        }
                        break;
                    case 3:
                        for (unsigned jj = j; jj < j + blockN/2; jj+=1 ) {
                            __m128d x, y;
                            x = in[jj + ii*N];
                            y = in[jj + (ii*N) + N/2];
                            out[ii + jj*M] = _mm_unpacklo_pd(x, y);
                            out[ii + (jj*M)+M/2] = _mm_unpackhi_pd(x, y);
                        }
                        break;
                }
            }
        }
    }
}


void vec_cpx_mul(const float* a, const float* b, float* d, int count) 
{
    count /= 2;    // count in double vectors
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int i = 0; i < count; ++i) {
        //_mm_prefetch(&a[(i + 16) * 4], _MM_HINT_T0);
        //_mm_prefetch(&b[(i + 16) * 4], _MM_HINT_T0);
        //_mm_prefetch(&d[(i + 16) * 4], _MM_HINT_T0);
        __m128 A, B, C, D;
        A = _mm_load_ps(&a[i * 4]);       // A = [ a0r a0i a1r a1i ]
        C = _mm_moveldup_ps(A);         // C = [ a0r a0r a1r a1r ]
        B = _mm_load_ps(&b[i * 4]);       // B = [ b0r b0i b1r b1i ]
        D = _mm_mul_ps(C, B);           // D = [ a0r*b0r a0r*b0i a1r*b1r a1r*b1i ]
        C = _mm_shuffle_ps(B, B, 0xB1); // C = [ b0i b0r b1i b1r ]
        A = _mm_movehdup_ps(A);         // A = [ a0i a0i a1i a1i ]
        A = _mm_mul_ps(A, C);           // A = [ a0i*b0i a0i*b0r a1i*b1i a1i*b1r ]
        D = _mm_addsub_ps(D, A);        // D = [ a0r*b0r-a0i*b0i a0r*b0i+a0i*b0r a1r*b1r-a1i*b1i a1r*b1i+a1i*b1r ]
        _mm_store_ps(&d[i * 4], D);
    }
}


void transpose_and_cmul_matrix(__m128d *in, __m128d *out, __m128d *mul, int M, int N) {

    static const int blockM = 128;
    static const int blockN = 4;
    ASSERT(!((M % blockM) || (N % blockN)));

    static const int LOOK_AHEAD = 8;


#define TACM_PROCESS(ii, jj) do {\
                    __m128d x,y,temp;\
                    __m128 xr,xi,yr,yi,temp1a,temp2a,temp3a,ma,temp1b,temp2b,temp3b,mb;\
                    x = in[jj + ii*N];\
                    y = in[jj + (ii*N) + N/2];\
                    ma = _mm_load_ps((const float *)&mul[ii + jj*M]);\
                    mb = _mm_load_ps((const float *)&mul[ii + jj*M + M/2]);\
                    temp = x;\
                    x = _mm_unpacklo_pd(x,y);\
                    y = _mm_unpackhi_pd(temp,y);\
                    xr = _mm_moveldup_ps((__m128)x);\
                    temp1a = _mm_mul_ps(xr, ma);\
                    temp3a = _mm_shuffle_ps(ma, ma, 0xB1);\
                    xi = _mm_movehdup_ps((__m128)x);\
                    temp2a = _mm_mul_ps(xi, temp3a);\
                    x = (__m128d)_mm_addsub_ps(temp1a, temp2a);\
                    out[ii + jj*M] = x;\
                    yr = _mm_moveldup_ps((__m128)y);\
                    temp3b = _mm_shuffle_ps(mb, mb, 0xB1);\
                    temp1b = _mm_mul_ps(yr, mb);\
                    yi = _mm_movehdup_ps((__m128)y);\
                    temp2b = _mm_mul_ps(yi, temp3b);\
                    y = (__m128d)_mm_addsub_ps(temp1b, temp2b);\
                    out[ii + (jj*M)+M/2] = y;\
                } while(0)

#define TACM_SECOND_PROCESS(ii, j) do {\
                _mm_prefetch(&in[j + (ii * N) + LOOK_AHEAD], _MM_HINT_T0);\
                _mm_prefetch(&in[j + (ii * N) + N/2 + LOOK_AHEAD], _MM_HINT_T0);\
                _mm_prefetch(&out[ii + ((j + LOOK_AHEAD) * M)], _MM_HINT_T0);\
                _mm_prefetch(&out[ii + ((j + LOOK_AHEAD) * M) + M/2], _MM_HINT_T0);\
                TACM_PROCESS(ii, j + 0);\
                TACM_PROCESS(ii, j + 1);\
            } while (0)

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int i = 0; i < M/2; i += blockM/2) {
        for (int j = 0; j < N/2; j += blockN/2) {
            for (int ii = i; ii < i + blockM/2; ii+=1)
                TACM_SECOND_PROCESS(ii, j);
        }
    }
}


void transpose_matrix(__m128d *in, __m128d *out, int M, int N, int blockM, int blockN) {
    ASSERT(!((M % blockM) || (N % blockN)));

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i < M/2; i += blockM/2) {
		for (int j = 0; j < N/2; j += blockN/2) {
			for (int ii = i; ii < i + blockM/2; ii+=1) {
				for (int jj = j; jj < j + blockN/2; jj+=1 ) {
					__m128d x, y;
					x = in[jj + ii*N];
					y = in[jj + (ii*N) + N/2];
					out[ii + jj*M] =  _mm_unpacklo_pd(x, y);
					out[ii + (jj*M)+M/2] = _mm_unpackhi_pd(x, y);
				}
			}
		}
	}
}
