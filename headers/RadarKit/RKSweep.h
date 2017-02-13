//
//  RKSweep.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Sweep__
#define __RadarKit_Sweep__

#include <RadarKit/RKFoundation.h>
#include <netcdf.h>

typedef uint32_t RKSweepEngineState;
enum RKSweepEngineState {
    RKSweepEngineStateNull           = 0,
    RKSweepEngineStateAllocated      = 1,
    RKSweepEngineStateActive         = (1 << 1),
    RKSweepEngineStateWritingFile    = (1 << 2)
};

typedef struct rk_sweep {
    RKRay                  *rays[RKMaxRaysPerSweep];
    uint32_t               rayCount;
} RKSweep;

typedef struct rk_sweep_engine RKSweepEngine;

struct rk_sweep_engine {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint32_t               configBufferDepth;
    RKBuffer               rayBuffer;
    uint32_t               *rayIndex;
    uint32_t               rayBufferDepth;
    uint8_t                verbose;
    bool                   doNotWrite;
    
    // Program set variables
    pthread_t              tidRayGatherer;
    RKSweep                sweep;
    float                  *array1D;
    float                  *array2D;
    
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
void RKSweepEngineSetInputOutputBuffer(RKSweepEngine *, RKRadarDesc *,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer rayBuffer,     uint32_t *rayIndex,    const uint32_t rayBufferDepth);
void RKSweepEngineSetDoNotWrite(RKSweepEngine *, const bool);

int RKSweepEngineStart(RKSweepEngine *);
int RKSweepEngineStop(RKSweepEngine *);

#endif /* RKSweep_h */
