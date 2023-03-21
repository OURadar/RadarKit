//
//  RKPositionSteerEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPositionSteerEngine.h>

// Internal functions

static void *positionSteerer(void *_in) {
    RKPositionSteerEngine *engine = (RKPositionSteerEngine *)_in;

    int k;

    RKLog("%s Started.   mem = %s B   positionIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->positionIndex);

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    k = 0;    // position index
    while (engine->state & RKEngineStateWantActive) {
        engine->tic++;

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
    }

    return NULL;
}

// Implementations

#pragma mark - Life Cycle

RKPositionSteerEngine *RKPositionSteerEngineInit(void) {
    RKPositionSteerEngine *engine = (RKPositionSteerEngine *)malloc(sizeof(RKPositionSteerEngine));
    memset(engine, 0, sizeof(RKPositionSteerEngine));
    sprintf(engine->name, "%s<PositionSteerer>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPositionSteerEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKPositionSteerEngine);
    engine->state = RKEngineStateAllocated;
    return engine;
}

void RKPositionSteerEngineFree(RKPositionSteerEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKPositionSteerEngineSetVerbose(RKPositionSteerEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKPositionSteerEngineSetInputOutputBuffers(RKPositionSteerEngine *engine, const RKRadarDesc *desc,
                                                RKPosition *positionBuffer, uint32_t *positionIndex,
                                                RKConfig   *configBuffer,   uint32_t *configIndex) {
    engine->radarDescription    = (RKRadarDesc *)desc;
    engine->positionBuffer      = positionBuffer;
    engine->positionIndex       = positionIndex;
    engine->configBuffer        = configBuffer;
    engine->configIndex         = configIndex;
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Interactions

int RKPositionSteerEngineStart(RKPositionSteerEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->threadId, NULL, positionSteerer, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartRingPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKPositionSteerEngineStop(RKPositionSteerEngine *engine) {
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
    if (engine->threadId) {
        pthread_join(engine->threadId, NULL);
        engine->threadId = (pthread_t)0;
    } else {
        RKLog("%s Error. Invalid thread ID = %p\n", engine->name, engine->threadId);
    }
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

char *RKPositionSteerEngineStatusString(RKPositionSteerEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
