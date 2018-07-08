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
#include <fftw3.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2);
float RKGetMinorSectorInDegrees(const float angle1, const float angle2);
float RKInterpolatePositiveAngles(const float angleBefore, const float angleAfter, const float alpha);
float RKInterpolateAngles(const float angleLeft, const float angleRight, const float alpha);

int RKMeasureNoiseFromPulse(RKFloat *noise, RKPulse *pulse, const int origin);
int RKBestStrideOfHops(const int hopCount, const bool showNumbers);

void RKHilbertTransform(RKFloat *x, RKComplex *y, const int n);

void RKFasterSineCosine(float x, float *sin, float *cos);
void RKFastSineCosine(float x, float *sin, float *cos);

//
// FIR + IIR Filters
//

typedef uint8_t RKFilterType;
enum RKFilterType {
    RKFilterTypeElliptical1,
    RKFilterTypeElliptical2,
    RKFilterTypeElliptical3,
    RKFilterTypeElliptical4,
    RKFilterTypeImpulse,
    RKFilterTypeTest1
};

void RKGetFilterCoefficients(RKIIRFilter *filter, const RKFilterType type);

// xcorr() ?
// ambiguity function
//


//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKDSP__) */
