//
//  RKGmap.h
//  RadarKit
//
//  Created by Skyler Garner in December 2025.
//  Copyright (c) 2017 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKGmap__
#define __RadarKit_RKGmap__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKScratch.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct rk_gmap RKGmap;


int RKGmapRun(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount);
RKGmap *RKGMapInit(void);
void RKGmapFree(RKGmap *gmap);
// float *make_fCorDots(int M);
// void cosine_window(float * w, unsigned n, const float * coeff, unsigned ncoeff, bool sflag);


typedef struct tWGCM    tWGCM;
typedef struct tDftConf tDftConf;

struct rk_gmap {
    // User set variables
    // interp

    // Program set variables
    tWGCM                           *wgcm;
    tDftConf                        *dftConf;

    // Status / health
};

#endif /* defined(__RadarKit_RKGmap__) */
