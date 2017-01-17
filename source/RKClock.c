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
    clock->yBuffer = (double *)malloc(clock->size * sizeof(double));
    clock->zBuffer = (double *)malloc(clock->size * sizeof(double));
    if (clock->tBuffer == NULL || clock->xBuffer == NULL || clock->uBuffer == NULL) {
        RKLog("Error. Unable to allocate internal buffers of an RKClock.\n");
        return NULL;
    }
    memset(clock->tBuffer, 0, clock->size * sizeof(struct timeval));
    memset(clock->xBuffer, 0, clock->size * sizeof(double));
    memset(clock->uBuffer, 0, clock->size * sizeof(double));
    memset(clock->yBuffer, 0, clock->size * sizeof(double));
    memset(clock->zBuffer, 0, clock->size * sizeof(double));
    struct timeval t;
    gettimeofday(&t, NULL);
    clock->initTime = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    clock->b = 1.0 / (double)clock->stride;
    clock->a = 1.0 - clock->b;
    return clock;
}

RKClock *RKClockInit(void) {
    return RKClockInitWithSize(RKClockDefaultBufferDepth, RKClockDefaultStride);
}

void RKClockFree(RKClock *clock) {
    char filename[64];
    sprintf(filename, "%s", clock->name + 1);
    filename[strlen(filename) - 1] = '\0';
    strcat(filename, ".csv");
    RKLog("%s Dumping buffers ... j = %d  %s\n", clock->name, clock->index, filename);
    FILE *fid = fopen(filename, "w");
    for (int i = 0; i < clock->size; i++) {
        fprintf(fid, "%.9f, %.9f, %.9f, %.9f\n", clock->xBuffer[i], clock->yBuffer[i], clock->uBuffer[i], clock->zBuffer[i]);
    }
    fclose(fid);
    free(clock->tBuffer);
    free(clock->xBuffer);
    free(clock->uBuffer);
    free(clock->yBuffer);
    free(clock->zBuffer);
    free(clock);
}

#pragma mark -
#pragma mark Properties

void RKClockSetName(RKClock *clock, const char *name) {
    strncpy(clock->name, name, RKNameLength - 1);
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
    clock->dx = dxdu;
    RKLog("%s received dx/du = %.2e as wisdom\n", clock->name, dxdu);
}

double RKClockGetTime(RKClock *clock, const double u, struct timeval *timeval) {
    int j, k;
    double x, dx, du, n;
    struct timeval t;
    
    // Get the time
    gettimeofday(&t, NULL);
    x = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    if (timeval) {
        *timeval = t;
    }
    if (x - clock->latestTime > RKClockAWhile) {
        clock->x0 = x;
        clock->u0 = u;
    }
    clock->latestTime = x;

    // Predict x0 and u0 using a running average, so we need to keep u's and x's.
    k = clock->index;
    clock->tBuffer[k] = t;
    clock->xBuffer[k] = x;
    clock->uBuffer[k] = u;
    if (clock->count > 1) {
        j = RKPreviousModuloS(k, clock->size);
        if (x - clock->xBuffer[j] > 2.0 || u - clock->uBuffer[j] < 0) {
            RKLog("%s Warning.   x = %s -> %s   u = %s -> %s",
                  clock->name,
                  RKFloatToCommaStyleString(clock->xBuffer[j]), RKFloatToCommaStyleString(x),
                  RKFloatToCommaStyleString(clock->uBuffer[j]), RKFloatToCommaStyleString(u));
        }
        if (clock->count > clock->stride) {
            j = RKPreviousNModuloS(k, clock->stride, clock->size);
            n = (double)clock->stride;
        } else {
            j = 0;
            n = (double)clock->count;
        }
        // Compute the gradient using a big stride
        dx = clock->xBuffer[k] - clock->xBuffer[j];
        du = clock->uBuffer[k] - clock->uBuffer[j];
        if (du <= 1.0e-6 || du > 1.0e9) {
            RKLog("Warning. Reference tic change of %.e per second is unexpected %.2e > %.2e\n", du, clock->count, clock->stride);
        }
        if (clock->b < 0.1 * dx / n) {
            RKLog("%s minor factor %.3e << %.3e may take a long time to converge.\n", clock->name, clock->b, dx / n);
            clock->b = 0.2 * dx / n;
            clock->a = 1.0 - clock->b;
            RKLog("%s updated to minor / major.   a = %.3e  b = %.3e", clock->name, clock->a, clock->b);
        }
        // Update the references as decaying function of the stride size
        clock->x0 = clock->a * clock->x0 + clock->b * x;
        clock->u0 = clock->a * clock->u0 + clock->b * u;
        clock->dx = clock->a * clock->dx + clock->b * dx / du;
        // Derive time using a linear relation
        x = clock->x0 + clock->dx * (u - clock->u0) + clock->offsetSeconds;
        if (!isfinite(x)) {
            x = 0.0;
        }
    }
    clock->yBuffer[k] = x;
    clock->zBuffer[k] = clock->dx;
    
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
