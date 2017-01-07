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

// Implementations

#pragma mark -
#pragma mark Helper Functions

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
    while (engine->clock->count < 2) {
        usleep(1000);
    }

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
            if (++s % 500 == 0 && engine->verbose) {
                RKLog("%s sleep 1/%d  k = %d   pulseIndex = %d   header.s = 0x%02x\n", engine->name, s, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        // Wait until it has data. Otherwise, time stamp may not be good.
        s = 0;
        while ((pulse->header.s & RKPulseStatusHasIQData) == 0 && engine->state == RKPositionEngineStateActive) {
            usleep(1000);
            if (++s % 500 == 0 && engine->verbose) {
                RKLog("%s sleep 2/%d  k = %d   pulseIndex = %d   header.s = 0x%02x\n", engine->name, s, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        // Wait until we have a position newer than pulse time.
        s = 0;
        while (engine->positionTimeLatest <= pulse->header.timeDouble && engine->state == RKPositionEngineStateActive) {
            usleep(1000);
            if (++s % 500 == 0 && engine->verbose) {
                RKLog("%s sleep 3/%d  k = %d   latestTime = %s <= %s = header.timeDouble\n", engine->name, s, k ,
                      RKFloatToCommaStyleString(engine->positionTimeLatest), RKFloatToCommaStyleString(pulse->header.timeDouble));
            }
        }
        if (engine->state == RKPositionEngineStateActive) {
            // Search until the time just after the pulse was acquired.
            // Then, roll back one slot, which should be the position just before the pulse was acquired.
            i = 0;
            while (engine->positionTime[j] <= pulse->header.timeDouble && i < engine->pulseBufferSize) {
                j = RKNextModuloS(j, RKPositionBufferSize);
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
            j = RKPreviousModuloS(j, RKPositionBufferSize);
            positionBefore = &engine->positionBuffer[j];   timeBefore = engine->positionTime[j];
            
            // Linear interpololation : V_interp = V_before + alpha * (V_after - V_before)
            alpha = (pulse->header.timeDouble - timeBefore) / (timeAfter - timeBefore);
            pulse->header.azimuthDegrees = RKInterpolateAngles(positionBefore->azimuthDegrees,
                                                               positionAfter->azimuthDegrees,
                                                               alpha);
            pulse->header.elevationDegrees = RKInterpolateAngles(positionBefore->elevationDegrees,
                                                                 positionAfter->elevationDegrees,
                                                                 alpha);
            
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
    sprintf(engine->name, "%s<pulseTagger>%s",
            rkGlobalParameters.showColor ? "\033[1;30;46m" : "", rkGlobalParameters.showColor ? RKNoColor : "");
    
    engine->clock = RKClockInit();
    RKClockSetName(engine->clock, "<positionClock>");

    engine->memoryUsage = sizeof(RKPositionEngine) + sizeof(RKClock);
    return engine;
}

void RKPositionEngineFree(RKPositionEngine *engine) {
    free(engine->clock);
    free(engine);
}

#pragma mark -
#pragma mark Properties

void RKPositionEngineSetVerbose(RKPositionEngine *engine, const int verbose) {
    engine->verbose = verbose;
    RKClockSetVerbose(engine->clock, verbose);
}

void RKPositionEngineSetInputOutputBuffers(RKPositionEngine *engine,
                                           RKBuffer buffer, uint32_t *index, const uint32_t size) {
    engine->pulseBuffer = buffer;
    engine->pulseIndex = index;
    engine->pulseBufferSize = size;
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
    if (engine->verbose) {
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
    RKPosition *position = &engine->positionBuffer[engine->positionIndex];
    position->flag = RKPositionFlagVacant;
    return position;
}

void RKPositionEngineSetPositionReady(RKPositionEngine *engine, RKPosition *position) {
    if (position->flag & ~RKPositionFlagHardwareMask) {
        RKLog("Error. Ingested position has a flag (0x%08x) outside of allowable value.\n", position->flag);
    }
    engine->positionTimeLatest = RKClockGetTime(engine->clock, (double)position->c, NULL);
    engine->positionTime[engine->positionIndex] = engine->positionTimeLatest;
    position->flag |= RKPositionFlagReady;
    engine->positionIndex = RKNextModuloS(engine->positionIndex, RKPositionBufferSize);
    if (engine->clock->count > RKPositionBufferSize) {
        engine->positionTimeOldest = engine->positionTime[engine->positionIndex];
    } else {
        engine->positionTimeOldest = engine->positionTime[0];
    }
}
