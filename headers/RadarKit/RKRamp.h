//
//  RKRamp.h
//  RadarKit
//
//  Created by Boonleng Cheong on 5/5/2021.
//  Copyright (c) Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Ramp__
#define __RadarKit_Ramp__

#include <RadarKit/RKFoundation.h>

typedef int RKRampType;
enum RKRampType{
    RKRampTypeZeros,
    RKRampTypeOnes,
    RKRampTypeStep,
    RKRampTypeLinear,
    RKRampTypeRaisedCosine
};

void RKRampMake(RKFloat *buffer, RKRampType type, const int length, ...);

#endif
