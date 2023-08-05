//
//  RKNoiseEstimator.h
//  RadarKit
//
//  Created by Min-Duan Tzeng on 7/26/2023.
//  Copyright (c) 2023 Min-Duan Tzeng. All rights reserved.
//

#ifndef __RadarKit_NoiseEstimator__
#define __RadarKit_NoiseEstimator__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKScratch.h>

int RKRayNoiseEstimator(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount);

#endif /* defined(__RadarKit_NoiseEstimator__) */
