//
//  RKClock.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKClock.h>

#pragma mark -
#pragma mark Life Cycle

RKClock *RKClockInit(void) {
    RKClock *clock = (RKClock *)malloc(sizeof(RKClock));
    if (clock == NULL) {
        RKLog("Error. Unable to alloccate an RKClock.\n");
        return NULL;
    }
    memset(clock, 0, sizeof(RKClock));
    struct timeval tv;
    gettimeofday(&tv, NULL);
    clock->initTime = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    for (int k = 0; k < RKClockBufferSize; k++) {
        clock->timeBuffer[k] = clock->initTime;
    }
    return clock;
}

void RKClockFree(RKClock *clock) {
    free(clock);
}

#pragma mark -
#pragma mark Properties

void RKClockSetOffset(RKClock *clock, double offset) {
    clock->offsetSeconds = offset;
}

double RKClockGetTime(RKClock *clock, const double input, struct timeval *timeval) {
    int j, k;
    double time;
    double period;
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    time = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    if (timeval) {
        *timeval = tv;
    }
    
    clock->count++;
    clock->tvBuffer[clock->index] = tv;
    clock->timeBuffer[clock->index] = time;
    clock->inputBuffer[clock->index] = input;
    clock->latestTime = time;
    
    if (clock->count > RKClockBufferSize) {
        // Subtract the oldest period (before it gets replaced as the newest)
        clock->accumulatedPeriod -= clock->periodBuffer[clock->index];
        
        // Assess the frequency of burst
        j = RKPreviousModuloS(clock->index, RKClockBufferSize);
        period = time - clock->timeBuffer[j];
        
        // Now add the newest period
        clock->accumulatedPeriod += period;
        clock->typicalPeriod = 1.0 / RKClockBufferSize * clock->accumulatedPeriod;;
        RKLog(">typicalPeriod = %.3f ms   mv = xxx ms\n", 1.0e3 * clock->typicalPeriod);
    }

//    // Do some math to get the actual time based on td & counter
//    double mv = time;
//    for (k = 0; k < 20; k++) {
//        mv += clock->periodBuffer[j];
//        j = RKPreviousModuloS(j, RKClockBufferSize);
//    }
//    mv /= (double)(k + 1);
    
    // Update the slot index for next call
    clock->index = RKNextModuloS(clock->index, RKClockBufferSize);
    return time;
}

double RKClockGetTimeSinceInit(RKClock *clock, const double time) {
    return time - clock->initTime;
}

#pragma mark -
#pragma mark Interactions
