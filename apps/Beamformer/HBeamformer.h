
#ifndef HBEAMFORMER_H
#define HBEAMFORMER_H
#pragma once
#include <complex>
#include <fftw3.h>
#include <xmmintrin.h>
#include <vector>

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

    void Run(const std::complex<float> *inptr, unsigned instride, std::complex<float> *outptr, unsigned outstride);

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
    void Stage3();
    /// workingData = ifft(workingData)
    void Stage4();
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

#endif
