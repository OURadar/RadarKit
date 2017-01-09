//
//  RKPosition.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPosition.h>

// Internal functions

void *pulseTagger(void *in);
void RKPositionnUpdateStatusString(RKPositionEngine *engine);

// Implementations

#pragma mark -
#pragma mark Helper Functions

void RKPositionnUpdateStatusString(RKPositionEngine *engine) {
    int i;
    char *string = engine->statusBuffer[engine->statusBufferIndex];
    // Full / compact string: Some spaces
    bool full = true;
    char spacer[2] = "";
    if (full) {
        sprintf(spacer, " ");
    }

    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';

    // Use b characters to draw a bar
    const int b = 10;
    i = engine->processedPulseIndex * (b + 1) / engine->pulseBufferSize;
    memset(string, '#', i);
    memset(string + i, '.', b - i);
    i = b + sprintf(string + b, "%s|", spacer);

    // Engine lag
    i += snprintf(string + i, RKMaximumStringLength - i, "%s%s%02.0f%s%s|",
                  spacer,
                  rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                  99.9f * engine->lag,
                  rkGlobalParameters.showColor ? RKNoColor : "",
                  spacer);

    // Almost Full flag
    i += snprintf(string + i, RKMaximumStringLength - i, " [%d]", engine->almostFull);
    if (i > RKMaximumStringLength - 13) {
        memset(string + i, '#', RKMaximumStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark -
#pragma mark Threads

void *pulseTagger(void *in) {
    RKPositionEngine *engine = (RKPositionEngine *)in;
    int i, j, k;
    RKPosition *positionBefore;
    RKPosition *positionAfter;
    double timeBefore;
    double timeAfter;
    double alpha;
    struct timeval t0, t1;

    // Search until a time I need

    // Wait until the latest position arrives
    // find the latest position, tag the pulse with the appropriate position
    // linearly interpolate between the best two readings
    // at some point, implement something sophisticated like Kalman filter

    if (engine->verbose) {
        RKLog("%s started.   mem = %s B   engine->index = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);
    }
    
    engine->state = RKPositionEngineStateActive;

    // Wait until there are at least two position readings.
    while (engine->clock->count < 2 && engine->state == RKPositionEngineStateActive) {
        usleep(1000);
    }

    gettimeofday(&t1, 0); t1.tv_sec -= 1;

    // Set the pulse to have position
    j = 0;   // position index
    k = 0;   // pulse index;
    int s = 0;
    while (engine->state == RKPositionEngineStateActive) {
        // Get the latest pulse
        RKPulse *pulse = RKGetPulse(engine->pulseBuffer, k);
        // Wait until a thread check out this pulse.
        while (k == *engine->pulseIndex && engine->state == RKPositionEngineStateActive) {
            usleep(1000);
            if (++s % 200 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        // Wait until it has data. Otherwise, time stamp may not be good.
        s = 0;
        while (!(pulse->header.s & RKPulseStatusHasIQData) && engine->state == RKPositionEngineStateActive) {
            usleep(1000);
            if (++s % 200 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        // Wait until we have a position newer than pulse time.
        s = 0;
        while (engine->positionTimeLatest <= pulse->header.timeDouble && engine->state == RKPositionEngineStateActive) {
            usleep(1000);
            if (++s % 200 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 3/%.1f s   k = %d   latestTime = %s <= %s = header.timeDouble\n",
                      engine->name, (float)s * 0.001f, k ,
                      RKFloatToCommaStyleString(engine->positionTimeLatest), RKFloatToCommaStyleString(pulse->header.timeDouble));
            }
        }
        if (engine->state == RKPositionEngineStateActive) {
            // Search until the time just after the pulse was acquired.
            // Then, roll back one slot, which should be the position just before the pulse was acquired.
            i = 0;
            while (engine->positionTime[j] <= pulse->header.timeDouble && i < engine->pulseBufferSize) {
                j = RKNextModuloS(j, engine->positionBufferSize);
                i++;
            }
            if (i == engine->pulseBufferSize) {
                RKLog("Could not find an appropriate position.  %.2f %s %.2f",
                      pulse->header.timeDouble,
                      pulse->header.timeDouble < engine->positionTimeLatest ? "<" : ">=",
                      engine->positionTimeLatest);
                continue;
            }
            positionAfter  = &engine->positionBuffer[j];   timeAfter  = engine->positionTime[j];
            j = RKPreviousModuloS(j, engine->positionBufferSize);
            positionBefore = &engine->positionBuffer[j];   timeBefore = engine->positionTime[j];
            
            // Linear interpololation : V_interp = V_before + alpha * (V_after - V_before)
            alpha = (pulse->header.timeDouble - timeBefore) / (timeAfter - timeBefore);
            pulse->header.azimuthDegrees = RKInterpolateAngles(positionBefore->azimuthDegrees,
                                                               positionAfter->azimuthDegrees,
                                                               alpha);
            pulse->header.elevationDegrees = RKInterpolateAngles(positionBefore->elevationDegrees,
                                                                 positionAfter->elevationDegrees,
                                                                 alpha);
            
            // Log a message if it has been a while
            gettimeofday(&t0, NULL);
            if (RKTimevalDiff(t0, t1) > 0.05) {
                t1 = t0;
                // Lag of the engine
                engine->processedPulseIndex = k;
                engine->lag = fmodf(((float)k + engine->pulseBufferSize - k) / engine->pulseBufferSize, 1.0f);
                RKPositionnUpdateStatusString(engine);
            }

            if (engine->verbose > 2) {
                RKLog("%s pulse[%llu] %llu time %.4f %s [%.4f] %s %.4f   az %.2f < [%.2f] < %.2f   el %.2f < [%.2f] < %.2f  %d\n",
                      engine->name,
                      pulse->header.i,
                      pulse->header.t,
                      RKClockGetTimeSinceInit(engine->clock, timeBefore),
                      timeBefore <= pulse->header.timeDouble ? "<" : ">=",
                      RKClockGetTimeSinceInit(engine->clock, pulse->header.timeDouble),
                      pulse->header.timeDouble <= timeAfter ? "<" : ">=",
                      RKClockGetTimeSinceInit(engine->clock, timeAfter),
                      positionBefore->azimuthDegrees,
                      pulse->header.azimuthDegrees,
                      positionAfter->azimuthDegrees,
                      positionBefore->elevationDegrees,
                      pulse->header.elevationDegrees,
                      positionAfter->elevationDegrees, engine->verbose);
            }
            
            pulse->header.s |= RKPulseStatusHasPosition;
        }
        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->pulseBufferSize);
    }
    return (void *)NULL;
}

#pragma mark -
#pragma mark Life Cycle

RKPositionEngine *RKPositionEngineInit() {
    RKPositionEngine *engine = (RKPositionEngine *)malloc(sizeof(RKPositionEngine));
    memset(engine, 0, sizeof(RKPositionEngine));
    sprintf(engine->name, "%s<pulsePositioner>%s",
            rkGlobalParameters.showColor ? "\033[1;97;46m" : "", rkGlobalParameters.showColor ? RKNoColor : "");
    
    engine->clock = RKClockInit();
    RKClockSetName(engine->clock, "<positionClock>");

    engine->memoryUsage = sizeof(RKPositionEngine) + sizeof(RKClock);
    return engine;
}

void RKPositionEngineFree(RKPositionEngine *engine) {
    free(engine->clock);
    free(engine->positionTime);
    free(engine);
}

#pragma mark -
#pragma mark Properties

void RKPositionEngineSetVerbose(RKPositionEngine *engine, const int verbose) {
    engine->verbose = verbose;
    RKClockSetVerbose(engine->clock, verbose);
}

void RKPositionEngineSetInputOutputBuffers(RKPositionEngine *engine,
                                           RKPosition *positionBuffer, uint32_t *positionIndex, const uint32_t positionBufferSize,
                                           RKPulse    *pulseBuffer,    uint32_t *pulseIndex,    const uint32_t pulseBufferSize) {
    engine->pulseBuffer = pulseBuffer;
    engine->pulseIndex = pulseIndex;
    engine->pulseBufferSize = pulseBufferSize;
    engine->positionBuffer = positionBuffer;
    engine->positionIndex = positionIndex;
    engine->positionBufferSize = positionBufferSize;
    engine->positionTime = (double *)malloc(positionBufferSize * sizeof(double));
    memset(engine->positionTime, 0, positionBufferSize * sizeof(double));
}

void RKPositionEngineSetHardwareInit(RKPositionEngine *engine, RKPedestal hardwareInit(void *), void *hardwareInitInput) {
    engine->hardwareInit = hardwareInit;
    engine->hardwareInitInput = hardwareInitInput;
}

void RKPositionEngineSetHardwareExec(RKPositionEngine *engine, int hardwareExec(RKPedestal, const char *)) {
    engine->hardwareExec = hardwareExec;
}

void RKPositionEngineSetHardwareFree(RKPositionEngine *engine, int hardwareFree(RKPedestal)) {
    engine->hardwareFree = hardwareFree;
}

#pragma mark -
#pragma mark Interactions

int RKPositionEngineStart(RKPositionEngine *engine) {
    if (engine->verbose) {
        RKLog("%s starting ...\n", engine->name);
    }
    if (pthread_create(&engine->threadId, NULL, pulseTagger, engine)) {
        RKLog("Error. Unable to start position engine.\n");
        return RKResultFailedToStartPedestalWorker;
    }
    while (engine->state < RKPositionEngineStateActive) {
        usleep(1000);
    }
    return RKResultNoError;
}

int RKPositionEngineStop(RKPositionEngine *engine) {
    if (engine->verbose > 1) {
        RKLog("%s stopping ...\n", engine->name);
    }
    engine->state = RKPositionEngineStateDeactivating;
    pthread_join(engine->threadId, NULL);
    if (engine->verbose) {
        RKLog("%s stopped.\n", engine->name);
    }
    return RKResultNoError;
}

RKPosition *RKPositionEngineGetVacantPosition(RKPositionEngine *engine) {
    RKPosition *position = &engine->positionBuffer[*engine->positionIndex];
    position->flag = RKPositionFlagVacant;
    return position;
}

void RKPositionEngineSetPositionReady(RKPositionEngine *engine, RKPosition *position) {
    if (position->flag & ~RKPositionFlagHardwareMask) {
        RKLog("Error. Ingested position has a flag (0x%08x) outside of allowable value.\n", position->flag);
    }
    engine->positionTimeLatest = RKClockGetTime(engine->clock, (double)position->c, NULL);
    engine->positionTime[*engine->positionIndex] = engine->positionTimeLatest;
    position->flag |= RKPositionFlagReady;
    *engine->positionIndex = RKNextModuloS(*engine->positionIndex, engine->positionBufferSize);
    if (engine->clock->count > engine->positionBufferSize) {
        engine->positionTimeOldest = engine->positionTime[*engine->positionIndex];
    } else {
        engine->positionTimeOldest = engine->positionTime[0];
    }
}

char *RKPositionEngineStatusString(RKPositionEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
