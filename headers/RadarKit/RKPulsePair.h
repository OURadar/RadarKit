//
//  RKPulsePair.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKPulsePair__
#define __RadarKit_RKPulsePair__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

int RKPulsePair(RKScratch *space, RKPulse **pulses, const uint16_t gateCount);
int RKPulsePairHop(RKScratch *space, RKPulse **pulses, const uint16_t gateCount);

#endif /* defined(__RadarKit_RKPulsePair__) */
