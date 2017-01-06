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

RKClock *RKClockInitWithSize(const uint32_t size, const uint32_t stride) {
    RKClock *clock = (RKClock *)malloc(sizeof(RKClock));
    if (clock == NULL) {
        RKLog("Error. Unable to alloccate an RKClock.\n");
        return NULL;
    }
    memset(clock, 0, sizeof(RKClock));
    
    // Copy over / set some non-zero parameters
    clock->size = size;
    clock->stride = stride;
    clock->autoSync = true;
    sprintf(clock->name, "<RKClock %p>", clock);
    clock->tvBuffer = (struct timeval *)malloc(clock->size * sizeof(struct timeval));
    clock->xBuffer = (double *)malloc(clock->size * sizeof(double));
    clock->uBuffer = (double *)malloc(clock->size * sizeof(double));
    if (clock->tvBuffer == NULL || clock->xBuffer == NULL || clock->uBuffer == NULL) {
        RKLog("Error. Unable to allocate internal buffers of an RKClock.\n");
        return NULL;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    clock->initTime = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    for (int k = 0; k < clock->size; k++) {
        clock->xBuffer[k] = clock->initTime;
    }
    return clock;
}

RKClock *RKClockInit(void) {
    return RKClockInitWithSize(RKClockDefaultBufferSize, RKClockDefaultStride);
}

void RKClockFree(RKClock *clock) {
    free(clock->uBuffer);
    free(clock->xBuffer);
    free(clock->tvBuffer);
    free(clock);
}

#pragma mark -
#pragma mark Properties

void RKClockSetName(RKClock *clock, const char *name) {
    strncpy(clock->name, name, RKMaximumStringLength - 1);
}

void RKClockSetVerbose(RKClock *clock, const int verbose) {
    clock->verbose = verbose;
}

void RKClockSetManualSync(RKClock *clock) {
    clock->autoSync = false;
}

void RKClockSetOffset(RKClock *clock, double offset) {
    clock->offsetSeconds = offset;
}

double RKClockGetTime(RKClock *clock, const double u, struct timeval *timeval) {
    int j, k = clock->index;
    double x, dx, du;
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    x = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    if (timeval) {
        *timeval = tv;
    }
    
    clock->count++;
    clock->tvBuffer[k] = tv;
    clock->xBuffer[k] = x;
    clock->uBuffer[k] = u;
    clock->latestTime = x;
    
    if (clock->count > clock->stride) {
        // Compute dx & du at stride samples apart
        if (clock->count % clock->stride == 0 && clock->autoSync) {
            j = RKPreviousNModuloS(k, clock->stride, clock->size);
            dx = clock->xBuffer[k] - clock->xBuffer[j];
            du = clock->uBuffer[k] - clock->uBuffer[j];
            if (du == 0.0) {
                RKLog("Error. Pulse tic is always the same.\n");
            }
            clock->x0 = clock->xBuffer[j];
            clock->u0 = clock->uBuffer[j];
            clock->dxdu = dx / du;
            if (clock->verbose) {
                RKLog("%s auto-synchronized.   period = %.2f ms   dx/du = %.2f ms  count = %ld   du = %.2e   dx = %.2e ms\n",
                      clock->name, 1.0e3 / (double)clock->stride * dx, 1.0e3 * clock->dxdu, clock->count, 1.0e3 * du, 1.0e3 * dx);
            }
            j = RKPreviousNModuloS(k, clock->stride, clock->size);
        }
        x = clock->x0 + clock->dxdu * (u - clock->u0) + clock->offsetSeconds;
    } else if (clock->count % (clock->stride / 10) == 0) {
        j = 0;
        dx = clock->xBuffer[k] - clock->xBuffer[j];
        du = clock->uBuffer[k] - clock->uBuffer[j];
        if (dx > 1.0e-2 && du > 0.0) {
            clock->x0 = clock->xBuffer[j];
            clock->u0 = clock->uBuffer[j];
            clock->dxdu = dx / du;
            RKLog("%s pre-synchronized.   period = %.2f ms   dx/du = %.2f ms\n",
                  clock->name, 1.0e3 / (double)clock->count * dx, 1.0e3 * clock->dxdu);
        }
    }
    if (clock->x0) {
        x = clock->x0 + clock->dxdu * (u - clock->u0) + clock->offsetSeconds;
    }
    if (clock->verbose > 2) {
        RKLog(">%s %d / %d   dx/du = %.2f ms   x = %.3f\n",
              clock->name, clock->index, clock->count,
              1.0e3 * clock->dxdu, RKClockGetTimeSinceInit(clock, x));
    }
    // Update the slot index for next call
    clock->index = RKNextModuloS(k, clock->size);
    return x;
}

double RKClockGetTimeSinceInit(RKClock *clock, const double time) {
    return time - clock->initTime;
}

#pragma mark -
#pragma mark Interactions
