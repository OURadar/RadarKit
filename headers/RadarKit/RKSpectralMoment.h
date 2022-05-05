//
//  RKSpectralMoment.h
//  RadarKit
//
//  Created by Boonleng Cheong on 10/8/18.
//  Copyright (c) 2018 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_SpectralMoment__
#define __RadarKit_SpectralMoment__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKPulsePair.h>

int RKSpectralMoment(RKScratch *space, RKPulse **pulses, const uint16_t pulseCount);
int RKSpectralMoment2(RKScratch *space, RKPulse **pulses, const uint16_t pulseCount);

#endif /* defined(__RadarKit_RSpectralMoment__) */
