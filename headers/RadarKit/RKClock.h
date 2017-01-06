//
//  RKClock.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKClock_h__
#define __RadarKit_RKClock_h__

#include <RadarKit/RKFoundation.h>

// A clock derived from counter and request time
#define RKClockBufferSize         1024

typedef struct rk_clock {
    // User set parameters
    double           offsetSeconds;
    uint16_t         depth;                                 // Maybe a user changeable depth? Let me think about this...

    // Program set parameters
    struct timeval   tvBuffer[RKClockBufferSize];           // The time which a request was made (dirty)
    double           timeBuffer[RKClockBufferSize];         // A ldouble representation of timeval
    double           inputBuffer[RKClockBufferSize];        // Clean driving reference
    double           periodBuffer[RKClockBufferSize];       // Period derived from dirty time
    
    uint32_t         index;
    uint64_t         count;
    double           initTime;
    double           latestTime;
    double           accumulatedPeriod;
    double           typicalPeriod;
    uint32_t         burstPeriod;
    
} RKClock;

RKClock *RKClockInit(void);
void RKClockFree(RKClock *);

void RKClockSetOffset(RKClock *, double);
double RKClockGetTime(RKClock *clock, const double, struct timeval *);
double RKClockGetTimeSinceInit(RKClock *clock, const double time);

#endif /* __RadarKit_RKClock_h__ */
