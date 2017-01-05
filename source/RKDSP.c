//
//  RKDSP.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKDSP.h>

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2) {
    float delta = angle1 - angle2;
    if (delta > 180.0f) {
        delta -= 360.0f;
    } else if (delta < -180.0f) {
        delta += 360.0f;
    }
    return delta;
}

float RKGetMinorSectorInDegrees(const float angle1, const float angle2) {
    return fabs(RKGetSignedMinorSectorInDegrees(angle1, angle2));
}

// Linear interpololation : V_interp = V_before + alpha * (V_after - V_before), alpha in [0, 1]
float RKInterpolateAngles(const float angleBefore, const float angleAfter, const float alpha) {
    float value = RKGetSignedMinorSectorInDegrees(angleAfter, angleBefore);
    value = angleBefore + alpha * value;
    if (value > 360.0f) {
        value -= 360.0f;
    } else if (value < 0.0f) {
        value += 360.0f;
    }
    return value;
}

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

double RKClockGetTime(RKClock *clock, const uint64_t counter, struct timeval *timeval) {
    int j, k;
    double td;
    double period;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    td = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    clock->count++;
    clock->counter[clock->index] = counter;
    clock->tvBuffer[clock->index] = tv;
    clock->tdBuffer[clock->index] = td;
    if (timeval) {
        *timeval = tv;
    }
    if (clock->count < RKClockBufferSize) {
        return td;
    }
    // Assess the frequency of burst
    j = RKPreviousModuloS(clock->index, RKClockBufferSize);
    period = td - clock->tdBuffer[j];
    clock->typicalPeriod = 0.25 * clock->typicalPeriod + 0.75 * period;
    
    // Do some math to get the actual time based on td & counter
    double mv = td;
    for (k = 0; k < 20; k++) {
        mv += clock->tdBuffer[j];
        j = RKPreviousModuloS(j, RKClockBufferSize);
    }
    mv /= (double)(k + 1);
    RKLog(">typicalPeriod = %.3f ms   mv = %.3f ms\n", 1.0e3 * clock->typicalPeriod, RKClockGetTimeSinceInit(clock, mv));
    
    // Update the slot index for next call
    clock->index = RKNextModuloS(clock->index, RKClockBufferSize);
    return td;
}

double RKClockGetTimeSinceInit(RKClock *clock, const double time) {
    return time - clock->initTime;
}

#pragma mark -
#pragma mark Interactions
