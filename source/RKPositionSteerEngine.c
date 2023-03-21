//
//  RKPositionSteerEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPositionSteerEngine.h>

// Internal functions

// Implementations

#pragma mark - Life Cycle

RKPositionSteerEngine *RKPositionSteerEngineInit(void) {
    return NULL;
}

void RKPositionSteerEngineFree(RKPositionSteerEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKPositionSteerEngineSetVerbose(RKPositionSteerEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKPositionSteerEngineSetInputOutputBuffers(RKPositionSteerEngine *, const RKRadarDesc *,
                                                RKPosition *, uint32_t *,
                                                RKConfig *,   uint32_t *);

#pragma mark - Interactions

int RKPositionSteerEngineStart(RKPositionSteerEngine *engine) {
    return 0;
}

int RKPositionSteerEngineStop(RKPositionSteerEngine *engine) {
    return 0;
}

char *RKPositionSteerEngineStatusString(RKPositionSteerEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
