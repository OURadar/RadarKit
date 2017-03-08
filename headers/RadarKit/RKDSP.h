//
//  RKDSP.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKDSP__
#define __RadarKit_RKDSP__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKSIMD.h>
#include <RadarKit/RKWindow.h>

//
// Z in [-32.0    ... 95.5]           Zi = (Z) x 2 + 64
// V in [-16.0    ... 15.875]         Vi = (V) x 8 + 128
// W in [ -0.05   ...  5.0]           Wi = (W) x 20 + 0
// D in [-10.0    ... 15.5]           Di = (D) x 10 + 100
// P in [  -PI    ... PI*(N-1)/N]     Pi = (P) x 128/M_PI + 128
// R in [  0.0    ... 1.079]
// K in [ -0.1*PI ... 0.1*PI*(N-1)/N] Ki = (K) x 1280/M_PI + 128
//
//
// Z  in [CLR  -31.50  ... 95.5]            Zi = (Z) * 2 + 64
// V  in [CLR  -15.875 ... 15.875]          Vi = (V) * 8 + 128
// W  in [CLR    0.00  ... 12.70]           Wi = (W) * 20 + 1
// D  in [CLR   -9.90  ... 15.5]            Di = (D) * 10 + 100
// P  in [CLR    -PI   ... +PI]             Pi = (P) * 255/(2*M_PI) + 128.5
// R  in [CLR   0.033  ... 1.079]
// K  in [CLR -0.1*PI  ... +0.1*PI]         Ki = (K) * 255/(20*M_PI) + 128.5
// VE in [CLR   -63.5  ... 63.5]            Vi = (V) * 2 + 128
//

#define RKZLHMAC  { lhma[0] = -32.0f;     lhma[1] = 95.5f;       lhma[2] = 2.0f;      lhma[3] =  64.0f; }  //
#define RKVLHMAC  { lhma[0] = -16.0f;     lhma[1] = 15.875f;     lhma[2] = 8.0f;      lhma[3] = 128.0f; }  //
#define RKWLHMAC  { lhma[0] = -0.05f;     lhma[1] = 12.70f;      lhma[2] = 20.0f;     lhma[3] =   1.0f; }  //
#define RKDLHMAC  { lhma[0] = -10.0f;     lhma[1] = 15.5f;       lhma[2] = 10.0f;     lhma[3] = 100.0f; }  //
#define RKPLHMAC  { lhma[0] = -3.16623f;  lhma[1] = 3.11695f;    lhma[2] = 40.5845f;  lhma[3] = 128.5f; }  //  pi - 2*pi/255
#define RKKLHMAC  { lhma[0] = -0.558508f; lhma[1] = 0.55414249f; lhma[2] = 229.1831f; lhma[3] = 128.0f; }  //  -0.2*pi + 0.4*pi/255
#define RKRLHMAC  { lhma[0] = 0.0f;       lhma[1] = 1.079f;      lhma[2] = 1.0f;      lhma[3] =   0.0f; }  //
#define RKV2LHMAC { lhma[0] = -32.0f;     lhma[1] = 31.75f;      lhma[2] = 4.0f;      lhma[3] = 128.0f; }  //
#define RKV3LHMAC { lhma[0] = -64.0f;     lhma[1] = 63.5f;       lhma[2] = 2.0f;      lhma[3] = 128.0f; }  //

#define RKRho2Uint8(r)    (r > 0.93f ? roundf((r - 0.93f) * 1000.0f) + 106.0f : (r > 0.7f ? roundf((r - 0.7f) * 300.0f) + 37.0f : roundf(r * 52.8571f)))

//#ifdef __cplusplus
//extern "C" {
//#endif

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2);
float RKGetMinorSectorInDegrees(const float angle1, const float angle2);
float RKInterpolatePositiveAngles(const float angleBefore, const float angleAfter, const float alpha);
float RKInterpolateAngles(const float angleLeft, const float angleRight, const float alpha);

//
// FIR + IIR Filters
//

// xcorr() ?
// ambiguity function
//


//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKDSP__) */
