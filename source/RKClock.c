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
    if (size < stride) {
        RKLog("Error. Clock size must be greater than stride.\n");
        return NULL;
    }
    RKClock *clock = (RKClock *)malloc(sizeof(RKClock));
    if (clock == NULL) {
        RKLog("Error. Unable to alloccate an RKClock.\n");
        return NULL;
    }
    memset(clock, 0, sizeof(RKClock));

    // Copy over / set some non-zero parameters
    clock->size = size;
	clock->block = 16;
    clock->stride = stride;
    clock->autoSync = true;
    clock->highPrecision = true;
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
    clock->initDay = floor(clock->initTime / 86400.0) * 86400.0;
    clock->b = 1.0 / (double)clock->stride;
    clock->a = 1.0 - clock->b;
    return clock;
}

RKClock *RKClockInit(void) {
    return RKClockInitWithSize(RKClockDefaultBufferDepth, RKClockDefaultStride);
}

void RKClockFree(RKClock *clock) {
#if defined(CLOCK_CSV)
    char *c, filename[64];
	c = strstr(clock->name, "<");
    sprintf(filename, "%s", c + 1);
	c = strstr(filename, ">");
    sprintf(c, ".csv");
    RKLog("%s Dumping buffers ... j = %d  %s\n", clock->name, clock->index, filename);
    FILE *fid = fopen(filename, "w");
    for (int i = 0; i < clock->size; i++) {
        fprintf(fid, "%.9f, %.9f, %.9f, %.9f\n", clock->xBuffer[i], clock->yBuffer[i], clock->uBuffer[i], clock->zBuffer[i]);
    }
    fclose(fid);
#endif
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
	if (clock->verbose) {
		RKLog("%s Received du/dx = %s as wisdom\n", clock->name, RKFloatToCommaStyleString(1.0 / dxdu));
	}
}

void RKClockSetDuDx(RKClock *clock, const double dudx) {
    return RKClockSetDxDu(clock, 1.0 / dudx);
}

void RKClockSetHighPrecision(RKClock *clock, const bool value) {
    clock->highPrecision = value;
}

//
// Important variables:
//   x - arrival time representation in double (noisy input)
//   u - reference that are correlated to time (clean source, tic count from controller)
//   t - predicted time
//
// NOTE:
//   Even with double precision, during the year of 2017, we are talking about > 1,500,000,000
//   seconds since 1970 Jan 1. So, there are only another 6 significant figures left, giving
//   us up to the precision of microseconds. If more than such precision is needed. The
//   reference time should be set to a running reference.
//
double RKClockGetTime(RKClock *clock, const double u, struct timeval *timeval) {
    int j = 0;
    double x, dx, du, y;
    struct timeval t;
    bool recent = true;
    
    // Get the time
    gettimeofday(&t, NULL);
    if (timeval) {
        *timeval = t;
    }
    // Pre-processing
    if (clock->highPrecision) {
        x = ((double)t.tv_sec - clock->initDay) + 1.0e-6 * (double)t.tv_usec;
    } else {
        x = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    }
    y = x;
    // Reset the references when clock count = 0 or it has been a while
    if (x - clock->latestTime > RKClockAWhile) {
        clock->count = 0;
        clock->index = 0;
    }
    // These parameters should be reset when the clock is restarted
    if (clock->count == 0) {
        recent = false;
        clock->x0 = x;
        clock->u0 = u;
        clock->sum_x0 = x;
        clock->sum_u0 = u;
    }
    // Predict x0 and u0 using a running average, so we need to keep u's and x's.
    const int k = clock->index;
    clock->tBuffer[k] = t;
    clock->xBuffer[k] = x;
    if (clock->useInternalReference) {
        clock->uBuffer[k] = clock->tic++;
    } else {
        clock->uBuffer[k] = u;
    }
    // We are done with the indices; update them for the next call
    clock->index = RKNextModuloS(k, clock->size);
    clock->count++;
    // Derive time only if the clock has been called at least once. Otherwise (1.0 / clock->count) would be undefined
    if (clock->count > 1) {
        // Quick check on the previous time to make sure we are still continuous
        j = RKPreviousModuloS(k, clock->size);
        if (x - clock->xBuffer[j] > 60.0 || u - clock->uBuffer[j] < 0) {
            // Latest in raw input xBuffer > latest (derived time > x)
            RKLog("%s Warning. dx = %s > %s   du = %s",
                  clock->name,
                  RKFloatToCommaStyleString(clock->xBuffer[j] - clock->latestTime), RKFloatToCommaStyleString(clock->latestTime - x),
                  RKFloatToCommaStyleString(clock->uBuffer[j] - u));
            if (x - clock->xBuffer[j] > RKClockAWhile) {
                recent = false;
            }
        }
        // Index of the reading from before of stride
        if (clock->count > clock->stride) {
            j = RKPreviousNModuloS(k, clock->stride, clock->size);
        } else {
            j = 0;
        }
        // Compute the gradient using a big stride
        dx = clock->xBuffer[k] - clock->xBuffer[j];
        du = clock->uBuffer[k] - clock->uBuffer[j];
        if (recent) {
            if (du <= 1.0e-7 || du > 1.0e9) {
                if (clock->tic == 0) {
                    RKLog("%s Warning. Reference tic change of %.e per second is unexpected.\n", clock->name, du);
                    RKLog("%s Warning. Will be replaced with an internal uniform reference.\n", clock->name);
                }
                // Override with own tic?
                clock->useInternalReference = true;
                clock->uBuffer[k] = clock->tic++;
                du = clock->uBuffer[k] - clock->uBuffer[j];
            }
            if (clock->count == clock->stride) {
                RKLog("%s b = %.2e   du/dx = %s   offset = %.3f ms", clock->name, clock->b, RKFloatToCommaStyleString(1.0 / clock->dx), 1.0e3 * clock->offsetSeconds);
            }
        }
        if (clock->count > clock->stride) {
            clock->sum_u0 = clock->sum_u0 + u - clock->uBuffer[j];
            clock->sum_x0 = clock->sum_x0 + x - clock->xBuffer[j];
            if (clock->count % 2 == 0) {
                clock->u0 = clock->sum_u0 / clock->stride;
                clock->x0 = clock->sum_x0 / clock->stride;
            }
            // Update dx / du as a decaying function
            clock->dx = clock->a * clock->dx + clock->b * dx / du;
        } else {
            clock->sum_u0 += u;
            clock->sum_x0 += x;
            if (clock->count % 2 == 0) {
                clock->u0 = clock->sum_u0 / clock->count;
                clock->x0 = clock->sum_x0 / clock->count;
            }
            // Update dx / du as a decaying function
            if (!clock->hasWisdom) {
                clock->dx = dx / du;
            }
        }
        // Derive time using a linear relation after it has tracked for a while
        y = clock->x0 + clock->dx * (u - clock->u0);
        if (!isfinite(y)) {
            y = 0.0;
        }
    }

#ifdef DEBUG_CLOCK
    
    printf("%s k = %d/%llu  j = %d   u = %.1f -> %.1f -> %.1f   x = %.2f -> %.1f -> %.3f   t = %.3f %s\n",
           clock->name, k, clock->count, j,
           u, clock->sum_u0, clock->u0,
           x, clock->sum_x0, clock->x0,
           y, clock->count > clock->stride ? "-" : "*");

#endif

    clock->yBuffer[k] = y;
    clock->zBuffer[k] = clock->dx;

	if (y < clock->latestTime && !recent) {
		RKLog("%s WARNING. Going back in time?  x = %f < %f = latestTime\n", clock->name, y, clock->latestTime);
	}
    clock->latestTime = y;

    return y + clock->offsetSeconds;
}

#pragma mark -
#pragma mark Interactions

void RKClockReset(RKClock *clock) {
    clock->index = 0;
    clock->count = 0;
    clock->tic = 0;
	clock->latestTime = 0;
	clock->infoShown = false;
	RKLog("%s Reset   du/dx = %s\n", clock->name, RKFloatToCommaStyleString(1.0 / clock->dx));
}
