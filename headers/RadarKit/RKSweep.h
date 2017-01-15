//
//  RKSweep.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKSweep__
#define __RadarKit_RKSweep__

#include <RadarKit/RKFoundation.h>

typedef uint32_t RKSweepEngineState;
enum RKSweepEngineState {
    RKSweepEngineStateNull           = 0,
    RKSweepEngineStateAllocated      = 1,
    RKSweepEngineStateActive         = (1 << 1),
    RKSweepEngineStateWritingFile    = (1 << 2)
};

typedef struct rk_sweep_writer RKSweepWriter;
typedef struct rk_sweep_engine RKSweepEngine;

struct rk_sweep_writer {
    char                   name[RKNameLength];
};

struct rk_sweep_engine {
    // User set variables
    char                   name[RKNameLength];
    RKBuffer               rayBuffer;
    uint32_t               *rayIndex;
    uint32_t               rayBufferDepth;
    uint8_t                verbose;
    
    // Program set variables
    pthread_t              tidRayGatherer;
    
    // Status / health
    uint32_t               processedRayIndex;
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKSweepEngineState     state;
    uint32_t               tic;
    float                  lag;
    uint32_t               almostFull;
    size_t                 memoryUsage;
};

RKSweepEngine *RKSweepEngineInit(void);
void RKSweepEngineFree(RKSweepEngine *);

void RKSweepEngineSetVerbose(RKSweepEngine *, const int verbose);
void RKSweepEngineSetInputBuffer(RKSweepEngine *, RKBuffer, uint32_t *rayIndex, const uint32_t rayBufferDepth);

int RKSweepEngineStart(RKSweepEngine *);
int RKSweepEngineStop(RKSweepEngine *);

#endif /* RKSweep_h */
