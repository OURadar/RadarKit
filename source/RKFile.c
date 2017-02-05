//
//  RKFile.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFile.h>

void RKFileEngineUpdateStatusString(RKFileEngine *engine) {
    int i;
    char *string;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';

    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * (RKStatusBarWidth + 1) / engine->pulseBufferDepth;
    memset(string, 'F', i);
    memset(string + i, '.', RKStatusBarWidth - i);

    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKMaximumStringLength - RKStatusBarWidth, " | %s%02.0f%s",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.9f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "");
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

void *pulseRecorder(void *in) {
    RKFileEngine *engine = (RKFileEngine *)in;

    int j, k, s;

    struct timeval t0, t1;

    RKPulse *pulse;
    RKConfig *config;

    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);

    gettimeofday(&t1, 0); t1.tv_sec -= 1;

    engine->state = RKFileEngineStateActive;

    j = 0;
    k = 0;
    while (engine->state == RKFileEngineStateActive) {
        // The pulse
        pulse = RKGetPulse(engine->pulseBuffer, k);
        // Wait until the buffer is advanced
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKFileEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->pulseIndex, pulse->header.s);
            }
        }
        // Wait until the pulse is completely processed
        while (!(pulse->header.s & RKPulseStatusProcessed) && engine->state == RKFileEngineStateActive) {
            usleep(1000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        if (engine->state != RKFileEngineStateActive) {
            break;
        }
        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->pulseBufferDepth - k) / engine->pulseBufferDepth, 1.0f);
        if (!isfinite(engine->lag)) {
            RKLog("%s %d + %d - %d = %d",
                  engine->name, *engine->pulseIndex, engine->pulseBufferDepth, k, *engine->pulseIndex + engine->pulseBufferDepth - k, engine->lag);
        }

        // Assess the configIndex
        if (j != pulse->header.configIndex) {
            j = pulse->header.configIndex;
            config = &engine->configBuffer[pulse->header.configIndex];

            // Close the current file
            RKLog("Closing file ...\n");
        }

        // Actual cache and write happen here.
        

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKFileEngineUpdateStatusString(engine);
        }

        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->pulseBufferDepth);
    }
    return NULL;
}


RKFileEngine *RKFileEngineInit(void) {
    RKFileEngine *engine = (RKFileEngine *)malloc(sizeof(RKFileEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate RKFileEngine.\r");
        exit(EXIT_FAILURE);
    }
    memset(engine, 0, sizeof(RKFileEngine));
    sprintf(engine->name, "%s<RawDataRecorder>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKFileEngineStateAllocated;
    engine->memoryUsage = sizeof(RKFileEngine);
    return engine;
}

void RKFileEngineFree(RKFileEngine *engine) {
    if (engine->state == RKFileEngineStateActive) {
        RKFileEngineStop(engine);
    }
    free(engine);
}

void RKFileEngineSetVerbose(RKFileEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKFileEngineSetInputOutputBuffers(RKFileEngine *engine,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth) {
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->configBufferDepth = configBufferDepth;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->pulseBufferDepth  = pulseBufferDepth;
}

int RKFileEngineStart(RKFileEngine *engine) {
    engine->state = RKFileEngineStateActivating;
    if (pthread_create(&engine->tidPulseRecorder, NULL, pulseRecorder, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartPulseRecorder;
    }
    while (engine->state < RKFileEngineStateActive) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKFileEngineStop(RKFileEngine *engine) {
    if (engine->state == RKFileEngineStateActive) {
        engine->state = RKFileEngineStateDeactivating;
        pthread_join(engine->tidPulseRecorder, NULL);
    } else {
        return RKResultEngineDeactivatedMultipleTimes;
    }
    engine->state = RKFileEngineStateSleep;
    return RKResultSuccess;
}

char *RKFileEngineStatusString(RKFileEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
