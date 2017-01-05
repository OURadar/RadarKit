//
//  RKDSP.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKDSP__
#define __RadarKit_RKDSP__

#include <RadarKit/RKFoundation.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

// A clock derived from counter and request time
#define RKClockBufferSize         1024

typedef struct rk_clock {
    struct timeval   tvBuffer[RKClockBufferSize];   // The time which a request was made
    double           tdBuffer[RKClockBufferSize];
    uint64_t         counter[RKClockBufferSize];
    double           offsetSeconds;

    // Some internal states or variables for time derivation
    double           initTime;
    uint32_t         index;
    uint64_t         count;
    double           typicalPeriod;
    uint32_t         burstPeriod;
    
} RKClock;

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2);
float RKGetMinorSectorInDegrees(const float angle1, const float angle2);
float RKInterpolateAngles(const float angleLeft, const float angleRight, const float alpha);

RKClock *RKClockInit(void);
void RKClockFree(RKClock *);

void RKClockSetOffset(RKClock *, double);
double RKClockGetTime(RKClock *clock, const uint64_t, struct timeval *);
double RKClockGetTimeSinceInit(RKClock *clock, const double time);

//
// FIR + IIR Filters
//

// xcorr() ?
// ambiguity function
//


//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKDSP__) */
