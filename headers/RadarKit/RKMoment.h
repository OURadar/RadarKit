//
//  RKMoment.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//
//

#ifndef __RadarKit_RKNetwork__
#define __RadarKit_RKNetwork__

#include <RadarKit/RKFoundation.h>

typedef void RKMomentProcessor;

typedef struct RKMoment {
    RKPulse        *pulses;
    RKFloatRay     *rawRays;
    RKInt16Ray     *encodedRays;

    uint32_t       pulseBufferSize;
    uint32_t       rayBufferSize;

    int            (*p)(RKMomentProcessor *);
} RKMoment;


#endif /* defined(___RadarKit_RKMoment__) */
