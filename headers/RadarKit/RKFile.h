//
//  RKFile.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKFile__
#define __RadarKit_RKFile__

#include <RadarKit/RKFoundation.h>

typedef int RKFileEngineState;
enum RKFileEngineState {
    RKFileEngineStateNull,
    RKFileEngineStateStateAllocated,
    RKFileEngineStateStateActivating,
    RKFileEngineStateStateActive,
    RKFileEngineStateStateDeactivating,
    RKFileEngineStateStateSleep
};

typedef struct rk_file_engine RKFileEngine;

struct rk_file_engine {
    // User set variables
    char                             name[RKNameLength];
    RKBuffer                         pulseBuffer;                        // Buffer of raw pulses
    uint32_t                         *pulseIndex;                        // The refence index to watch for
    uint32_t                         pulseBufferDepth;                   // Size of the buffer
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    uint32_t                         configBufferDepth;
    uint8_t                          verbose;

    // Status / health
    char                             statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t                         statusBufferIndex;
    RKFileEngineState                state;
    uint32_t                         tic;
    float                            lag;
    size_t                           memoryUsage;
};

RKFileEngine *RKFileEngineInit(void);
void RKFileEngineFree(RKFileEngineState *engine);

void RKFileEngineSetVerbose(RKFileEngine *, const int);
void RKFileEngineSetInputOutputBuffers(RKFileEngine *engine,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth);
int RKFileEngineStart(RKFileEngine *engine);
int RKFileEngineStop(RKFileEngine *engine);
char *RKFileEngineStatusString(RKFileEngine *engine);

#endif /* defined(__RadarKit_RKFile__) */
