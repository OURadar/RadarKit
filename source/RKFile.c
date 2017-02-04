//
//  RKFile.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFile.h>

void *pulseRecorder(void *in) {
    RKFileEngine *engine = (RKFileEngine *)in;

    engine->state = RKFileEngineStateActive;

    return NULL;
}


RKFileEngine *RKFileEngineInit(void) {
    RKFileEngine *engine = (RKFileEngine *)malloc(sizeof(RKFileEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate RKFileEngine.\r");
        exit(EXIT_FAILURE);
    }
    memset(engine, 0, sizeof(RKFileEngine));
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
