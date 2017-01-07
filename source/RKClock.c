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
    clock->tBuffer = (struct timeval *)malloc(clock->size * sizeof(struct timeval));
    clock->xBuffer = (double *)malloc(clock->size * sizeof(double));
    clock->uBuffer = (double *)malloc(clock->size * sizeof(double));
    if (clock->tBuffer == NULL || clock->xBuffer == NULL || clock->uBuffer == NULL) {
        RKLog("Error. Unable to allocate internal buffers of an RKClock.\n");
        return NULL;
    }
    struct timeval t;
    gettimeofday(&t, NULL);
    clock->initTime = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    for (int k = 0; k < clock->size; k++) {
        clock->xBuffer[k] = clock->initTime;
    }
    return clock;
}

RKClock *RKClockInit(void) {
    return RKClockInitWithSize(RKClockDefaultBufferSize, RKClockDefaultStride);
}

void RKClockFree(RKClock *clock) {
    free(clock->tBuffer);
    free(clock->xBuffer);
    free(clock->uBuffer);
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

void RKClockSetDxDu(RKClock *clock, const double dxdu) {
    clock->hasWisdom = true;
    clock->dxdu = dxdu;
    RKLog("%s received dxdu = %s as wisdom\n", clock->name, RKFloatToCommaStyleString(dxdu));
}

double RKClockGetTime(RKClock *clock, const double u, struct timeval *timeval) {
    int j;
    int k = clock->index;
    double x, dx, du;
    struct timeval t;
    
    // Get the time
    gettimeofday(&t, NULL);
    x = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    if (timeval) {
        *timeval = t;
    }
    if (x - clock->latestTime > RKClockAWhile) {
        RKClockSync(clock, u);
    }
    clock->latestTime = x;
    
    if (clock->hasWisdom) {
        // Derive time using a simplified Kalman filter, covariance of u and time is identity
        x = clock->x0 + clock->dxdu * (u - clock->u0) + clock->offsetSeconds;
    } else {
        // Predict x0 and u0 using a running average, so we need to keep u's and x's.
        clock->tBuffer[k] = t;
        clock->xBuffer[k] = x;
        clock->uBuffer[k] = u;
        if (clock->autoSync && clock->count % clock->stride == 0) {
            // Compute dx & du at (clock->stride) samples apart
            j = RKPreviousNModuloS(k, clock->stride, clock->size);
            dx = clock->xBuffer[k] - clock->xBuffer[j];
            du = clock->uBuffer[k] - clock->uBuffer[j];
            if (du <= 1.0e-6 || du > 1.0e9) {
                RKLog("Error. Pulse tic change of %.e per second is unexpected.\n", du);
            }
            clock->x0 = clock->xBuffer[j];
            clock->u0 = clock->uBuffer[j];
            clock->dxdu = dx / du;
            if (clock->verbose > 1) {
                RKLog("%s auto-sync.   period = %.2f ms   dx/du = %.3e ms  count = %ld   du = %.2e   dx = %.2e ms\n",
                      clock->name, 1.0e3 / (double)clock->stride * dx, 1.0e3 * clock->dxdu, clock->count, 1.0e3 * du, 1.0e3 * dx);
            }
        } else if (clock->count < clock->stride && clock->count % (clock->stride / 4) == 0) {
            j = 0;
            dx = clock->xBuffer[k] - clock->xBuffer[j];
            du = clock->uBuffer[k] - clock->uBuffer[j];
            if (dx > 1.0e-2 && du > 0.0) {
                clock->x0 = clock->xBuffer[j];
                clock->u0 = clock->uBuffer[j];
                clock->dxdu = dx / du;
                if (clock->verbose > 1) {
                    RKLog("%s pre-sync.   period = %.2f ms   dx/du = %.2f ms\n",
                          clock->name, 1.0e3 / (double)clock->count * dx, 1.0e3 * clock->dxdu);
                }
            }
        }
        if (clock->count > (clock->stride / 4)) {
            x = clock->x0 + clock->dxdu * (u - clock->u0) + clock->offsetSeconds;
        }
    }
    
    if (clock->verbose > 2) {
        RKLog(">%s %d / %d   dx/du = %.2f ms   x = %.3f\n",
              clock->name, clock->index, clock->count,
              1.0e3 * clock->dxdu, RKClockGetTimeSinceInit(clock, x));
    }
    // Update the slot index for next call
    clock->index = RKNextModuloS(k, clock->size);
    clock->count++;
    return x;
}

double RKClockGetTimeSinceInit(RKClock *clock, const double time) {
    return time - clock->initTime;
}

#pragma mark -
#pragma mark Interactions

// This function resets the reference of x0 and u0 but keeps dxdu the same
void RKClockSync(RKClock *clock, const double u) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double x = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    clock->x0 = x;
    clock->u0 = u;
    clock->latestTime = x;
    RKLog("%s sync  u = %s   u0 = %s   x0 = %s",
          clock->name, RKFloatToCommaStyleString(u), RKFloatToCommaStyleString(clock->u0), RKFloatToCommaStyleString(clock->x0));
}
