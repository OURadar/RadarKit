//
//  RKDSP.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_DSP__
#define __RadarKit_DSP__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKSIMD.h>
#include <RadarKit/RKWindow.h>

#define RKCommonFFTPlanCount 18

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct rk_fft_resource {
    unsigned int                     size;
    unsigned int                     count;
    fftwf_plan                       forwardInPlace;
    fftwf_plan                       forwardOutPlace;
    fftwf_plan                       backwardInPlace;
    fftwf_plan                       backwardOutPlace;
} RKFFTResource;

typedef struct rk_fft_module {
    RKName                           name;
    int                              verbose;
    bool                             exportWisdom;
    char                             wisdomFile[64];
    unsigned int                     count;
    RKFFTResource                    plans[RKCommonFFTPlanCount];
} RKFFTModule;

typedef struct rk_gaussian {
    RKFloat                          A;                                        //
    RKFloat                          mu;                                       //
    RKFloat                          sigma;                                    //
} RKGaussian;

//
// A scratch space for pulse compression
//
typedef struct rk_compression_scratch {
    RKBuffer                         pulse;                                    //
    RKComplex                        *filter;                                  //
    RKFilterAnchor                   *filterAnchor;                            //
    fftwf_plan                       planForwardInPlace;                       //
    fftwf_plan                       planForwardOutPlace;                      //
    fftwf_plan                       planBackwardInPlace;                      //
    fftwf_plan                       planBackwardOutPlace;                     //
    fftwf_complex                    *inBuffer;                                //
    fftwf_complex                    *outBuffer;                               //
    RKIQZ                            *zi;                                      //
    RKIQZ                            *zo;                                      //
    unsigned int                     planSize;                                 //
} RKCompressionScratch;

//
// A scratch space for moment processor
//
typedef struct rk_moment_scratch {
    uint32_t                         capacity;                                 // Capacity
    bool                             showNumbers;                              // A flag for showing numbers
    uint8_t                          userLagChoice;                            // Number of lags in multi-lag estimator from user
    uint8_t                          lagCount;                                 // Number of lags of R & C
    uint16_t                         gateCount;                                // Gate count of the rays
    RKFloat                          gateSizeMeters;                           // Gate size in meters for range correction
    RKFloat                          samplingAdjustment;                       // Sampling adjustment going from pulse to ray conversion
    RKIQZ                            mX[2];                                    // Mean of X, 2 for dual-pol
    RKIQZ                            vX[2];                                    // Variance of X, i.e., E{X' * X} - E{X}' * E{X}
    RKIQZ                            R[2][RKMaximumLagCount];                  // ACF up to RKMaximumLagCount - 1 for each polarization
    RKIQZ                            C[2 * RKMaximumLagCount - 1];             // CCF in [ -RKMaximumLagCount + 1, ..., -1, 0, 1, ..., RKMaximumLagCount - 1 ]
    RKIQZ                            sC;                                       // Summation of Xh * Xv'
    RKIQZ                            ts;                                       // Temporary scratch space
    RKFloat                          *aR[2][RKMaximumLagCount];                // abs(ACF)
    RKFloat                          *aC[2 * RKMaximumLagCount - 1];           // abs(CCF)
    RKFloat                          *gC;                                      // Gaussian fitted CCF(0)  NOTE: Need to extend this to multi-multilag
    RKFloat                          noise[2];                                 // Noise floor of each channel
    RKFloat                          velocityFactor;                           // Velocity factor to multiply by atan2(R(1))
    RKFloat                          widthFactor;                              // Width factor to multiply by the ln(S/|R(1)|) :
    RKFloat                          KDPFactor;                                // Normalization factor of 1.0 / gateWidth in kilometers
    RKFloat                          *dcal;                                    // Calibration offset to D
    RKFloat                          *pcal;                                    // Calibration offset to P (radians)
    RKFloat                          SNRThreshold;                             // SNR threshold for masking
    RKFloat                          SQIThreshold;                             // SQI threshold for masking
    RKFloat                          *rcor[2];                                 // Reflectivity range correction factor
    RKFloat                          *S[2];                                    // Signal
    RKFloat                          *Z[2];                                    // Reflectivity in dB
    RKFloat                          *V[2];                                    // Velocity in same units as aliasing velocity
    RKFloat                          *W[2];                                    // Spectrum width in same units as aliasing velocity
    RKFloat                          *Q[2];                                    // Signal quality index SQI
    RKFloat                          *SNR[2];                                  // Signal-to-noise ratio
    RKFloat                          *ZDR;                                     // Differential reflectivity ZDR
    RKFloat                          *PhiDP;                                   // Differential phase PhiDP
    RKFloat                          *RhoHV;                                   // Cross-correlation coefficient RhoHV
    RKFloat                          *KDP;                                     // Specific phase KDP
    uint8_t                          *mask;                                    // Mask for censoring
    RKFFTModule                      *fftModule;                               // A reference to the common FFT module
    fftwf_complex                    **inBuffer;                               //
    fftwf_complex                    **outBuffer;                              //
    int8_t                           fftOrder;                                 // FFT order that was used to perform FFT. This will be copied over to rayHeader
} RKScratch;

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2);
float RKGetMinorSectorInDegrees(const float angle1, const float angle2);
float RKInterpolatePositiveAngles(const float angleBefore, const float angleAfter, const float alpha);
float RKInterpolateAngles(const float angleLeft, const float angleRight, const float alpha);

int RKMeasureNoiseFromPulse(RKFloat *noise, RKPulse *pulse, const int origin);
int RKBestStrideOfHopsV1(const int hopCount, const bool showNumbers);
int RKBestStrideOfHops(const int hopCount, const bool showNumbers);

void RKHilbertTransform(RKFloat *x, RKComplex *y, const int n);

void RKFasterSineCosine(float x, float *sin, float *cos);
void RKFastSineCosine(float x, float *sin, float *cos);

//
// FIR + IIR Filters
//

void RKGetFilterCoefficients(RKIIRFilter *filter, const RKFilterType type);

//
// Common FFT plans
//

RKFFTModule *RKFFTModuleInit(const uint32_t capacity, const int verb);
void RKFFTModuleFree(RKFFTModule *);

// xcorr() ?
// ambiguity function
//

RKGaussian RKSGFit(RKFloat *x, RKComplex *y, const int count);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKDSP__) */
