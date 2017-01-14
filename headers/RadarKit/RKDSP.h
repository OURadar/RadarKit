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

#define ZLHCMAC  { lhc[0] = -32.0f;     lhc[1] = 95.5f;       mac[0] = 2.0f;      mac[1] =  64.0f; }
#define VLHCMAC  { lhc[0] = -16.0f;     lhc[1] = 15.875f;     mac[0] = 8.0f;      mac[1] = 128.0f; }
#define WLHCMAC  { lhc[0] = -0.05f;     lhc[1] = 12.70f;      mac[0] = 20.0f;     mac[1] =   1.0f; }
#define DLHCMAC  { lhc[0] = -10.0f;     lhc[1] = 15.5f;       mac[0] = 10.0f;     mac[1] = 100.0f; }
#define PLHCMAC  { lhc[0] = -3.16623f;  lhc[1] = 3.11695f;    mac[0] = 40.5845f;  mac[1] = 128.5f; }  // -pi - 2*pi/255
#define KLHCMAC  { lhc[0] = -0.558508f; lhc[1] = 0.55414249f; mac[0] = 229.1831f; mac[1] = 128.0f; }  // -0.2*pi + 0.4*pi/255
#define RLHCMAC  { lhc[0] = 0.0f;       lhc[1] = 1.079f;      mac[0] = 1.0f;      mac[1] =   0.0f; }

#define RHO2CHAR(r)    (unsigned char)(r > 0.93f ? roundf((r - 0.93f) * 1000.0f) + 106.0f : (r > 0.7f ? roundf((r - 0.7f) * 300.0f) + 37.0f : roundf(r * 52.8571f)))
//#define RHO2CHAR(r)  (unsigned char)(roundf((1.0f - sqrtf(1.0f - r)) * 255.0f))

#define V2LHCMAC { lhc[0] = -32.0f;    lhc[1] = 31.75f;      mac[0] = 4.0f;      mac[1] = 128.0f; }
#define V3LHCMAC { lhc[0] = -64.0f;    lhc[1] = 63.5f;       mac[0] = 2.0f;      mac[1] = 128.0f; }



//#ifdef __cplusplus
//extern "C" {
//#endif

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2);
float RKGetMinorSectorInDegrees(const float angle1, const float angle2);
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
