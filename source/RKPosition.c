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

#pragma mark - Helper Functions

#define RKPositionAzimuthFlagColor(x)                           \
(x & RKPositionFlagAzimuthError ? "\033[91m" :                  \
(x & RKPositionFlagAzimuthEnabled ? "\033[92m" : "\033[91m"))

#define RKPositionElevationFlagColor(x)                         \
(x & RKPositionFlagElevationError ? "\033[91m" :                \
(x & RKPositionFlagElevationEnabled ? "\033[92m" : "\033[91m"))

void RKPositionnUpdateStatusString(RKPositionEngine *engine) {
    int i, k;
    char *string;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];
    
    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';

    // Use b characters to draw a bar
    const int b = 10;
    k = engine->processedPulseIndex * (b + 1) / engine->pulseBufferSize;
    memset(string, '#', k);
    memset(string + k, '.', b - k);
    i = b + sprintf(string + b, " | ");

    // Engine lag
    i += snprintf(string + i, RKMaximumStringLength - i, "%s%02.0f%s |",
                  rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                  99.9f * engine->lag,
                  rkGlobalParameters.showColor ? RKNoColor : "");
    
    // Position string
    string = engine->positionStringBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';

    // Same as previous status, use b characters to draw a bar
    k = *engine->positionIndex * (b + 1) / engine->positionBufferSize;
    memset(string, '#', k);
    memset(string + k, '.', b - k);
    i = b + sprintf(string + b, " %04d |", *engine->positionIndex);
    RKPosition *position = &engine->positionBuffer[RKPreviousModuloS(*engine->positionIndex, engine->positionBufferSize)];
    i += snprintf(string + i,RKMaximumStringLength - i, " %010llu  %sAZ%s %6.2f° @ %+7.2f°/s   %sEL%s %6.2f° @ %+6.2f°/s",
                  (unsigned long long)position->c,
                  rkGlobalParameters.showColor ? RKPositionAzimuthFlagColor(position->flag) : "",
                  rkGlobalParameters.showColor ? RKNoColor : "",
                  position->azimuthDegrees, position->azimuthVelocityDegreesPerSecond,
                  rkGlobalParameters.showColor ? RKPositionElevationFlagColor(position->flag) : "",
                  rkGlobalParameters.showColor ? RKNoColor : "",
                  position->elevationDegrees, position->elevationVelocityDegreesPerSecond);

    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Threads

void *pulseTagger(void *in) {
    RKPositionEngine *engine = (RKPositionEngine *)in;
    
    int i, j, k, s;
    
    RKPulse *pulse;
    RKPosition *positionBefore;
    RKPosition *positionAfter;
    double timeBefore;
    double timeAfter;
    double timeLatest;
    double alpha;
    struct timeval t0, t1;
    RKMarker marker0, marker1;
    bool hasSweepComplete;

    RKLog("%s started.   mem = %s B   engine->index = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);
    
    engine->state = RKPositionEngineStateActive;
    
    // If multiple workers are needed, here will be the time to launch them.

    gettimeofday(&t1, 0); t1.tv_sec -= 1;
    marker1 = RKMarkerSweepEnd;

    // Wait until there is something ingested
    s = 0;
    while (*engine->positionIndex < 2) {
        usleep(1000);
        if (++s % 200 == 0 && engine->verbose > 1) {
            RKLog("%s sleep 0/%.1f s\n",
                  engine->name, (float)s * 0.001f);
        }
    }
    
    // Set the pulse to have position
    j = 0;   // position index
    k = 0;   // pulse index;
    s = 0;   // sleep counter
    while (engine->state == RKPositionEngineStateActive) {
        // Get the latest pulse
        pulse = RKGetPulse(engine->pulseBuffer, k);
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
        i = RKPreviousModuloS(*engine->positionIndex, RKBufferPSlotCount);
        while (engine->positionBuffer[i].timeDouble <= pulse->header.timeDouble && engine->state == RKPositionEngineStateActive) {
            usleep(1000);
            if (++s % 200 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 3/%.1f s   k = %d   latestTime = %s <= %s = header.timeDouble\n",
                      engine->name, (float)s * 0.001f, k ,
                      RKFloatToCommaStyleString(engine->positionBuffer[i].timeDouble), RKFloatToCommaStyleString(pulse->header.timeDouble));
            }
            i = RKPreviousModuloS(*engine->positionIndex, RKBufferPSlotCount);
        }
        
        // Record down the latest time
        timeLatest = engine->positionBuffer[i].timeDouble;
        
        if (engine->state == RKPositionEngineStateActive) {
            // Lag of the engine
            engine->lag = fmodf(((float)*engine->pulseIndex + engine->pulseBufferSize - k) / engine->pulseBufferSize, 1.0f);
            
            // Search until the time just after the pulse was acquired.
            i = 0;
            hasSweepComplete = false;
            while (engine->positionBuffer[j].timeDouble <= pulse->header.timeDouble && i < engine->pulseBufferSize) {
                hasSweepComplete |= engine->positionBuffer[j].flag & (RKPositionFlagAzimuthComplete | RKPositionFlagElevationComplete);
                j = RKNextModuloS(j, engine->positionBufferSize);
                i++;
            }
            if (i == engine->pulseBufferSize) {
                RKLog("Could not find an appropriate position.  %.2f %s %.2f",
                      pulse->header.timeDouble,
                      pulse->header.timeDouble < timeLatest ? "<" : ">=",
                      timeLatest);
                continue;
            }
            positionAfter  = &engine->positionBuffer[j];   timeAfter  = positionAfter->timeDouble;
            
            // Roll back one slot, which should be the position just before the pulse was acquired.
            j = RKPreviousModuloS(j, engine->positionBufferSize);
            positionBefore = &engine->positionBuffer[j];   timeBefore = positionBefore->timeDouble;
            
            // Linear interpololation : V_interp = V_before + alpha * (V_after - V_before)
            alpha = (pulse->header.timeDouble - timeBefore) / (timeAfter - timeBefore);
            pulse->header.azimuthDegrees = RKInterpolateAngles(positionBefore->azimuthDegrees,
                                                               positionAfter->azimuthDegrees,
                                                               alpha);
            pulse->header.elevationDegrees = RKInterpolateAngles(positionBefore->elevationDegrees,
                                                                 positionAfter->elevationDegrees,
                                                                 alpha);
            // Consolidate markers from the positions
            marker0 = RKMarkerNull;
            
            // First set of logics are purely from position
            if (positionBefore->flag & RKPositionFlagActive) {
                marker0 |= RKMarkerSweepMiddle;
            }
            if ((positionBefore->flag & RKPositionFlagElevationPoint) && (positionBefore->flag & RKPositionFlagAzimuthSweep)) {
                marker0 |= RKMarkerPPIScan;
            } else if ((positionBefore->flag & RKPositionFlagAzimuthPoint) && (positionBefore->flag & RKPositionFlagElevationSweep)) {
                marker0 |= RKMarkerRHIScan;
            } else if ((positionBefore->flag & RKPositionFlagAzimuthPoint) && (positionBefore->flag & RKPositionFlagElevationPoint)) {
                marker0 |= RKMarkerPointScan;
            }
            
            // Second set of logics are derived from marker change
            // NOTE: hasComplete indicates that a sweep complete has been encountered during the search, which may be prior to positionBefore
            // NOTE: i = 1 means this loop is still using the same pair of positionBefore and positionAfter
            //       i = 2 means the next pair was valid, this is the case when pulses come in equal or faster than positions
            //       i > 2 means the next pair was invalid, this is the case when pulses come in slower than positions
            if (i == 2 && (positionBefore->flag & (RKPositionFlagAzimuthComplete | RKPositionFlagElevationComplete))) {
                marker0 |= RKMarkerSweepEnd;
            } else if (i > 2 && hasSweepComplete && !(marker1 & RKMarkerSweepEnd)) {
                marker0 |= RKMarkerSweepEnd;
            }
            if (!(marker1 & RKMarkerSweepMiddle) && (marker0 & RKMarkerSweepMiddle)) {
                marker0 |= RKMarkerSweepBegin;
            }
            if ((marker1 & RKMarkerSweepMiddle) && !(marker0 & RKMarkerSweepMiddle)) {
                marker0 |= RKMarkerSweepEnd;
            }
            if ((marker1 & RKMarkerSweepEnd) && (marker0 & RKMarkerSweepMiddle)) {
                marker0 |= RKMarkerSweepBegin;
            }
//            if ((marker0 & RKMarkerSweepEnd) || (marker0 & RKMarkerSweepBegin) || positionBefore->flag & RKMarkerSweepEnd) {
//                RKLog(">%s %d %.2f 0x%08x < 0x%08x < 0x%08x   %d %d\n",
//                      engine->name, pulse->header.i, pulse->header.azimuthDegrees,
//                      positionBefore->flag,
//                      marker0,
//                      positionAfter->flag,
//                      hasSweepComplete, i);
//            }
            
            pulse->header.marker = marker0;
            marker1 = marker0;
            
            // Log a message if it has been a while
            gettimeofday(&t0, NULL);
            if (RKTimevalDiff(t0, t1) > 0.05) {
                t1 = t0;
                engine->processedPulseIndex = k;
                RKPositionnUpdateStatusString(engine);
            }

            if (engine->verbose > 2) {
                RKLog("%s pulse[%04llu]  T [ %.4f %s %.4f %s %.4f ]   A [ %7.2f < %7.2f < %7.2f ]   E [ %.2f < %+7.2f < %+7.2f ]  %d %08x < \033[3%dm%08x\033[0m < %08x (%d / %d)\n",
                      engine->name,
                      pulse->header.i,
                      timeBefore - engine->startTime,
                      timeBefore <= pulse->header.timeDouble ? "<" : ">=",
                      pulse->header.timeDouble - engine->startTime,
                      pulse->header.timeDouble <= timeAfter ? "<" : ">=",
                      timeAfter - engine->startTime,
                      positionBefore->azimuthDegrees,
                      pulse->header.azimuthDegrees,
                      positionAfter->azimuthDegrees,
                      positionBefore->elevationDegrees,
                      pulse->header.elevationDegrees,
                      positionAfter->elevationDegrees,
                      (int)hasSweepComplete,
                      positionBefore->flag,
                      marker0 & 0x7,
                      marker0,
                      positionAfter->flag,
                      i, j);
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
    sprintf(engine->name, "%s<PulsePositioner>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKPositionEngine) + sizeof(RKClock);
    return engine;
}

void RKPositionEngineFree(RKPositionEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKPositionEngineSetVerbose(RKPositionEngine *engine, const int verbose) {
    engine->verbose = verbose;
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

#pragma mark - Interactions

int RKPositionEngineStart(RKPositionEngine *engine) {
    RKLog("%s starting ...\n", engine->name);
    if (pthread_create(&engine->threadId, NULL, pulseTagger, engine)) {
        RKLog("Error. Unable to start position engine.\n");
        return RKResultFailedToStartPedestalWorker;
    }
    while (engine->state < RKPositionEngineStateActive) {
        usleep(10000);
    }
    struct timeval t;
    gettimeofday(&t, NULL);
    engine->startTime = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    return RKResultNoError;
}

int RKPositionEngineStop(RKPositionEngine *engine) {
    if (engine->verbose > 1) {
        RKLog("%s stopping ...\n", engine->name);
    }
    engine->state = RKPositionEngineStateDeactivating;
    pthread_join(engine->threadId, NULL);
    RKLog("%s stopped.\n", engine->name);
    engine->state = RKPositionEngineStateNull;
    return RKResultNoError;
}

char *RKPositionEngineStatusString(RKPositionEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

char *RKPositionEnginePositionString(RKPositionEngine *engine) {
    return engine->positionStringBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
