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

#pragma mark -
#pragma mark Threads

void *pulseTagger(void *in) {
    RKPositionEngine *engine = (RKPositionEngine *)in;
    
    int i, j, k;
    
    RKPulse *pulse;
    RKPosition *positionBefore;
    RKPosition *positionAfter;
    double timeBefore;
    double timeAfter;
    double timeLatest;
    double alpha;
    struct timeval t0, t1;

    RKLog("%s started.   mem = %s B   engine->index = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);
    
    engine->state = RKPositionEngineStateActive;
    
    // If multiple workers are needed, here will be the time to launch them.

    gettimeofday(&t1, 0); t1.tv_sec -= 1;

    // Set the pulse to have position
    j = 0;   // position index
    k = 0;   // pulse index;
    int s = 0;
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
            while (engine->positionBuffer[j].timeDouble <= pulse->header.timeDouble && i < engine->pulseBufferSize) {
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
            
            // Log a message if it has been a while
            gettimeofday(&t0, NULL);
            if (RKTimevalDiff(t0, t1) > 0.05) {
                t1 = t0;
                engine->processedPulseIndex = k;
                RKPositionnUpdateStatusString(engine);
            }

            if (engine->verbose > 2) {
                RKLog("%s pulse[%llu] %llu time %.4f %s [%.4f] %s %.4f   az %.2f < [%.2f] < %.2f   el %.2f < [%.2f] < %.2f  %d\n",
                      engine->name,
                      pulse->header.i,
                      pulse->header.t,
                      timeBefore,
                      timeBefore <= pulse->header.timeDouble ? "<" : ">=",
                      pulse->header.timeDouble,
                      pulse->header.timeDouble <= timeAfter ? "<" : ">=",
                      timeAfter,
                      positionBefore->azimuthDegrees,
                      pulse->header.azimuthDegrees,
                      positionAfter->azimuthDegrees,
                      positionBefore->elevationDegrees,
                      pulse->header.elevationDegrees,
                      positionAfter->elevationDegrees,
                      engine->verbose);
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
    //
    //  http://misc.flogisoft.com/bash/tip_colors_and_formatting
    //
    sprintf(engine->name, "%s<pulsePositioner>%s",
            rkGlobalParameters.showColor ? "\033[1;97;46m" : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKPositionEngine) + sizeof(RKClock);
    return engine;
}

void RKPositionEngineFree(RKPositionEngine *engine) {
    free(engine);
}

#pragma mark -
#pragma mark Properties

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

#pragma mark -
#pragma mark Interactions

int RKPositionEngineStart(RKPositionEngine *engine) {
    RKLog("%s starting ...\n", engine->name);
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
