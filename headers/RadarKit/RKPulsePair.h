//
//  RKPulsePair.h
//  RadarKit
//
//  Created by Boonleng Cheong on 10/31/16.
//  Copyright (c) 2016 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_PulsePair__
#define __RadarKit_PulsePair__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

int RKPulsePair(RKScratch *, RKPulse **, const uint16_t);
int RKPulsePairHop(RKScratch *, RKPulse **, const uint16_t);
int RKPulsePairStaggeredPRT(RKScratch *, RKPulse **, const uint16_t);

#endif /* defined(__RadarKit_RKPulsePair__) */
