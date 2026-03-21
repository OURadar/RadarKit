//
//  RKGMAP.h
//  RadarKit
//
//  Created by Skyler Garner in December 2025.
//  Copyright (c) 2017 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKGMAP__
#define __RadarKit_RKGMAP__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKScratch.h>

typedef struct tWGCM    tWGCM;
typedef struct tDftConf tDftConf;

int RKGMAPRun(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount);

#endif /* defined(__RadarKit_RKGMAP__) */
