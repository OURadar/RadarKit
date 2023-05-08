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
#include <RadarKit/RKScratch.h>

int RKPulsePair(RKMomentScratch *, RKPulse **, const uint16_t);
int RKPulsePairHop(RKMomentScratch *, RKPulse **, const uint16_t);
int RKPulsePairStaggeredPRT(RKMomentScratch *, RKPulse **, const uint16_t);

#endif /* defined(__RadarKit_RKPulsePair__) */
