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
    int k;
    uint32_t j = 0;

//    // Search until a time I need
//
//    // Wait until the latest position arrives
//    // find the latest position, tag the pulse with the appropriate position
//    // linearly interpolate between the best two readings
//    // at some point, implement something sophisticated like Kalman filter
//
    if (engine->verbose) {
        RKLog("<pulseTagger> started.   mem = %s B   engine->index = %d\n", RKIntegerToCommaStyleString(engine->memoryUsage), j);
    }

    engine->state = RKPositionEngineStateActive;

    RKPosition *positionBefore;
    RKPosition *positionAfter;
    double timeBefore;
    double timeAfter;
    double alpha;
    
    // Set the pulse to have position
    uint32_t pulseIndex = *engine->pulseIndex;
    while (engine->state == RKPositionEngineStateActive) {
        while (pulseIndex == *engine->pulseIndex) {
            usleep(1000);
        }
        // Get the latest pulse
        RKPulse *pulse = RKGetPulse(engine->pulseBuffer, pulseIndex);

        // If we do not have a position newer than pulse time, just wait a little.
        while (engine->positionTimeLatest < pulse->header.timeDouble) {
            usleep(1000);
        }
        // Search until the time just after the pulse was acquired.
        // Then, roll back one slot, which should be the position just before the pulse was acquired.
        k = 0;
        while (engine->positionTime[j] <= pulse->header.timeDouble && k < engine->pulseBufferSize) {
            j = RKNextModuloS(j, RKPositionBufferSize);
            k++;
        }
        //printf("Tagging pulse %d / %d    %d...\n", pulse->header.i, pulseIndex, engine->positionIndex);
        if (k == engine->pulseBufferSize) {
            RKLog("Could not find an appropriate position. All readings have expired.");
            continue;
        }
        positionAfter  = &engine->positionBuffer[j];   timeAfter  = engine->positionTime[j];
        j = RKPreviousModuloS(j, RKPositionBufferSize);
        positionBefore = &engine->positionBuffer[j];   timeBefore = engine->positionTime[j];
        
        // Linear interpololation : V_mid = V_before + alpha * (V_after - V_before)
        alpha = (pulse->header.timeDouble - timeBefore) / (timeAfter - timeBefore);
        pulse->header.azimuthDegrees = RKInterpolateAngles(positionBefore->azimuthDegrees,
                                                           positionAfter->azimuthDegrees,
                                                           alpha);
        pulse->header.elevationDegrees = RKInterpolateAngles(positionBefore->elevationDegrees,
                                                             positionAfter->elevationDegrees,
                                                             alpha);
        
        if (engine->verbose > 2) {
            RKLog("pulse time since init : %.4f %s [%.4f] %s %.4f   az %.2f < [%.2f] < %.2f   el %.2f < [%.2f] < %.2f  %d\n",
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
        
        pulseIndex = RKNextModuloS(pulseIndex, engine->pulseBufferSize);
    }
    if (engine->verbose) {
        RKLog("<pulseTagger> stopped.\n");
    }
    return (void *)NULL;
}

#pragma mark -
#pragma mark Life Cycle

RKPositionEngine *RKPositionEngineInit() {
    RKPositionEngine *engine = (RKPositionEngine *)malloc(sizeof(RKPositionEngine));
    memset(engine, 0, sizeof(RKPositionEngine));
    engine->clock = RKClockInit();
    for (int k = 0; k < RKPositionBufferSize; k++) {
        engine->positionTime[k] = RKClockGetTime(engine->clock, 0);
    }
    engine->memoryUsage = sizeof(RKPositionEngine) + sizeof(RKClock);
    return engine;
}

void RKPositionEngineFree(RKPositionEngine *engine) {
    free(engine->clock);
    free(engine);
}

#pragma mark -
#pragma mark Properties

void RKPositionEngineSetVerbose(RKPositionEngine *engine, const int verb) {
    engine->verbose = verb;
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
    RKLog("<pulseTagger> starting ...\n");
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
    RKLog("<pulseTagger> stopping ...\n");
    engine->state = RKPositionEngineStateDeactivating;
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
    engine->positionTimeLatest = RKClockGetTime(engine->clock, position->c);
    engine->positionTime[engine->positionIndex] = engine->positionTimeLatest;
    position->flag |= RKPositionFlagReady;
    engine->positionIndex = RKNextModuloS(engine->positionIndex, RKPositionBufferSize);
}
