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
    RKLog("%s Received dx/du = %.2e as wisdom\n", clock->name, dxdu);
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
double RKClockGetTime(RKClock *clock, const double u, struct timeval *timeval) {
    int j, k;
    double x, dx, du;
    struct timeval t;
    bool recent = true;
    
    // Get the time
    gettimeofday(&t, NULL);
    if (timeval) {
        *timeval = t;
    }
    if (clock->highPrecision) {
        x = ((double)t.tv_sec - clock->initDay) + 1.0e-6 * (double)t.tv_usec;
    } else {
        x = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    }
    // Reset the references when clock count = 0 or it has been a while
    if (clock->count == 0 || x - clock->latestTime > RKClockAWhile) {
        recent = false;
        clock->x0 = x;
        clock->u0 = u;
        // We are missing clock->dx here. Perhaps use the supplied wisdom?
    }
    // Predict x0 and u0 using a running average, so we need to keep u's and x's.
    k = clock->index;
    clock->tBuffer[k] = t;
    clock->xBuffer[k] = x;
    clock->uBuffer[k] = u;
    if (clock->count > 1) {
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
        if (clock->count < clock->stride) {
			j = 0;
        } else {
			j = RKPreviousNModuloS(k, clock->stride, clock->size);
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
                clock->uBuffer[k] = clock->tic++;
                du = clock->uBuffer[k] - clock->uBuffer[j];
            }
//			if (clock->count > 3 * clock->stride) {
//				if (clock->b < 0.002 * dx / (double)clock->stride) {
//					RKLog("%s minor factor %.3e << %.3e may take a long time to converge.\n", clock->name, clock->b, 0.002 * dx / (double)clock->stride);
//					clock->b = 0.002 * dx / (double)clock->stride;
//					clock->a = 1.0 - clock->b;
//					RKLog("%s updated to minor / major.   a = %.4e   b = %.4e", clock->name, clock->a, clock->b);
//					RKClockReset(clock);
//					return x;
//				} else if (clock->b > 5.0 * dx / (double)clock->stride) {
//					RKLog("%s The reading can be smoother with lower minor factor. dx / n = %.2e --> %.2e\n", clock->name, clock->b, 0.1 * dx / (double)clock->stride);
//					clock->b = 0.1 * dx / (double)clock->stride;
//					clock->a = 1.0 - clock->b;
//					RKLog("%s updated to minor / major.   a = %.4e   b = %.4e", clock->name, clock->a, clock->b);
//					RKClockReset(clock);
//					return x;
//				} else if (!clock->infoShown) {
//					clock->infoShown = true;
//					RKLog("%s b = %.2e vs %.2e", clock->name, clock->b, 0.1 * dx / (double)clock->stride);
//				}
//			}
        }
//        // Update the references as decaying function of the stride size
//		clock->x0 = clock->a * clock->x0 + clock->b * x;
//		clock->u0 = clock->a * clock->u0 + clock->b * u;
//		clock->dx = clock->a * clock->dx + clock->b * dx / du;
//		if (clock->count > 3 * clock->stride) {
//			// Derive time using a linear relation after it has tracked for a while
//			x = clock->x0 + clock->dx * (u - clock->u0);
//			if (!isfinite(x)) {
//				x = 0.0;
//			}
//		}
		if (clock->count < clock->block) {
			clock->x0 = x;
			clock->u0 = (u - clock->uBuffer[0]) / clock->count;
			clock->dx = clock->a * (clock->xBuffer[k - 1] - clock->xBuffer[0]) / (clock->uBuffer[k - 1] - clock->uBuffer[0]) + clock->b * dx / du;
			printf("%s k = %d / %lu   x0 = %.3e  u0 = %.3e\n", clock->name, k, clock->count, clock->x0, clock->u0);
		} else {
			clock->x0 = clock->a * clock->x0 + clock->b * x;
			clock->u0 = clock->a * clock->u0 + clock->b * u;
			clock->dx = clock->a * clock->dx + clock->b * dx / du;
		}
		// Derive time using a linear relation after it has tracked for a while
		x = clock->x0 + clock->dx * (u - clock->u0);
		if (!isfinite(x)) {
			x = 0.0;
		}
    }
    clock->yBuffer[k] = x;
    clock->zBuffer[k] = clock->dx;

	if (x < clock->latestTime && !recent) {
		RKLog("%s WARNING. Going back in time?  x = %f < %f = latestTime\n", clock->name, x, clock->latestTime);
	}
    clock->latestTime = x;

    // Update the slot index for next call
    clock->index = RKNextModuloS(k, clock->size);
    clock->count++;
    return x + clock->offsetSeconds;
}

#pragma mark -
#pragma mark Interactions

void RKClockReset(RKClock *clock) {
    clock->index = 0;
    clock->count = 0;
    clock->tic = 0;
	clock->latestTime = 0;
	clock->infoShown = false;
	RKLog("%s Reset\n", clock->name);
}
