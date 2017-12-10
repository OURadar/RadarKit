//
//  RKMultiLag.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/2/17.
//  Copyright (c) 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_MultiLag__
#define __RadarKit_MultiLag__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

int RKMultiLag(RKScratch *space, RKPulse **pulses, const uint16_t pulseCount);

#endif /* defined(__RadarKit_RKMultiLag__) */
