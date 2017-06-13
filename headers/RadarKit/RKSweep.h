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
#include <RadarKit/RKFileManager.h>
#include <netcdf.h>

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
    bool                   hasHandleFilesScript;
    bool                   handleFilesScriptProducesTgz;
    bool                   handleFilesScriptProducesZip;
    char                   handleFilesScript[RKMaximumPathLength];
    RKFileManager          *fileManager;

    // Program set variables
    pthread_t              tidRayGatherer;
    RKSweep                sweep;
    float                  *array1D;
    float                  *array2D;
    char                   filelist[RKMaximumStringLength];              // It's really handleFilesScript + file list
    char                   filename[RKMaximumPathLength];
    char                   productName[RKNameLength];
    char                   productUnit[RKNameLength];
    char                   productColormap[RKNameLength];
    char                   summary[RKMaximumStringLength];

    // Status / health
    uint32_t               processedRayIndex;
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    uint32_t               tic;
    float                  lag;
    uint32_t               almostFull;
    size_t                 memoryUsage;
};

RKSweepEngine *RKSweepEngineInit(void);
void RKSweepEngineFree(RKSweepEngine *);

void RKSweepEngineSetVerbose(RKSweepEngine *, const int verbose);
void RKSweepEngineSetInputOutputBuffer(RKSweepEngine *, RKRadarDesc *, RKFileManager *,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer rayBuffer,     uint32_t *rayIndex,    const uint32_t rayBufferDepth);
void RKSweepEngineSetDoNotWrite(RKSweepEngine *, const bool);
void RKSweepEngineSetHandleFilesScript(RKSweepEngine *engine, const char *script, const bool expectTgz);

int RKSweepEngineStart(RKSweepEngine *);
int RKSweepEngineStop(RKSweepEngine *);

#endif /* RKSweep_h */
