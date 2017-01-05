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

#pragma mark -

RKClock *RKClockInit(void) {
    RKClock *clock = (RKClock *)malloc(sizeof(RKClock));
    if (clock == NULL) {
        RKLog("Error. Unable to alloccate an RKClock.\n");
        return NULL;
    }
    memset(clock, 0, sizeof(RKClock));
    return clock;
}

void RKClockSetOffset(RKClock *clock, double offset) {
    clock->offsetSeconds = offset;
}

double RKClockGetTime(RKClock *clock, struct timeval *time, const uint64_t counter) {
    struct timeval tv;
    double td;

    gettimeofday(&tv, NULL);
    td = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    clock->counter[clock->index] = counter;
    clock->tvBuffer[clock->index] = tv;
    clock->tdBuffer[clock->index] = td;
    if (clock->count < RKClockBufferSize) {
        *time = tv;
        return td;
    }

    clock->count++;
    return td;
}
