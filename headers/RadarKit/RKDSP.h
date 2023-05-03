//
//  RKDSP.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/18/15.
//  Copyright (c) 2015 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_DSP__
#define __RadarKit_DSP__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKSIMD.h>
#include <RadarKit/RKWindow.h>
#include <RadarKit/RKRamp.h>

#define RKCommonFFTPlanCount 18

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
    RKFloat                          A;                                            //
    RKFloat                          mu;                                           //
    RKFloat                          sigma;                                        //
} RKGaussian;

//
// A scratch space for pulse compression
//
typedef struct rk_compression_scratch {
    RKName                           name;                                         //
    uint8_t                          verbose;                                      //
    RKPulse                          *pulse;                                       //
    RKComplex                        *filter;                                      // (deprecating)
    RKFilterAnchor                   *filterAnchor;                                // (deprecating)
    fftwf_plan                       planForwardInPlace;                           //
    fftwf_plan                       planForwardOutPlace;                          //
    fftwf_plan                       planBackwardInPlace;                          //
    fftwf_plan                       planBackwardOutPlace;                         //
    fftwf_complex                    *inBuffer;                                    //
    fftwf_complex                    *outBuffer;                                   //
    RKIQZ                            *zi;                                          //
    RKIQZ                            *zo;                                          //
    unsigned int                     planSize;                                     // DFT plan size
    RKConfig                         *config;                                      //
    RKComplex                        **arrays;                                     // Array of arrays
    uint16_t                         *arraySizes;                                  // Array sizes
    uint16_t                         waveformGroupdId;                             // Index of RKConfig->waveform to use
    uint16_t                         waveformFilterId;                             // Index of RKConfig->waveform->filterAnchor to use
    void                             *userResource;                                //
} RKCompressionScratch;

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

//
// Half, Single, and Double Precision Floats
//

RKWordFloat64 RKSingle2Double(const RKWordFloat32 x);
RKWordFloat32 RKHalf2Single(const RKWordFloat16 x);

#endif /* defined(__RadarKit_RKDSP__) */
