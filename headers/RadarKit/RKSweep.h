//
//  RKSweep.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Sweep__
#define __RadarKit_Sweep__

#define RKRayAnchorsDepth 4

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>
#include <netcdf.h>

typedef struct rk_ray_anchors {
    RKRay                            *rays[RKMaximumRaysPerSweep];
    uint32_t                         count;
} RKRayAnchors;

typedef struct rk_sweep_engine RKSweepEngine;

struct rk_sweep_engine {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    RKBuffer                         rayBuffer;
    uint32_t                         *rayIndex;
    uint8_t                          verbose;
    bool                             doNotWrite;
    bool                             hasHandleFilesScript;
    bool                             handleFilesScriptProducesTgz;
    bool                             handleFilesScriptProducesZip;
    char                             handleFilesScript[RKMaximumPathLength];
    RKFileManager                    *fileManager;
    uint32_t                         userProductTimeoutSeconds;

    // Program set variables
    pthread_t                        tidRayGatherer;
    RKRayAnchors                     rayAnchors[RKRayAnchorsDepth];
	uint8_t                          rayAnchorsIndex;
    float                            *array1D;
    float                            *array2D;
    char                             filelist[RKMaximumStringLength];              // It's really handleFilesScript + file list
    char                             filename[RKMaximumPathLength];
	char                             productSymbol[8];
    RKName                           productName;
    RKName                           productUnit;
    RKName                           productColormap;
    char                             summary[RKMaximumStringLength];
    RKUserProduct                    userProducts[RKMaximumUserProductCount];

    // Status / health
    uint32_t                         processedRayIndex;
    char                             statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t                         statusBufferIndex;
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    uint32_t                         almostFull;
    size_t                           memoryUsage;
};

RKSweepEngine *RKSweepEngineInit(void);
void RKSweepEngineFree(RKSweepEngine *);

void RKSweepEngineSetVerbose(RKSweepEngine *, const int verbose);
void RKSweepEngineSetInputOutputBuffer(RKSweepEngine *, RKRadarDesc *, RKFileManager *,
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKBuffer rayBuffer,     uint32_t *rayIndex);
void RKSweepEngineSetDoNotWrite(RKSweepEngine *, const bool);
void RKSweepEngineSetUserProductTimeout(RKSweepEngine *, const uint32_t);
void RKSweepEngineSetHandleFilesScript(RKSweepEngine *engine, const char *script, const bool expectTgz);

int RKSweepEngineStart(RKSweepEngine *);
int RKSweepEngineStop(RKSweepEngine *);

RKUserProductId RKSweepEngineRegisterProduct(RKSweepEngine *, RKUserProductDesc);
int RKSweepEngineUnregisterProduct(RKSweepEngine *, RKUserProductId);
int RKSweepEngineReportProduct(RKSweepEngine *, RKFloat *, RKUserProductId);

RKSweep *RKSweepCollect(RKSweepEngine *, const uint8_t);
RKSweep *RKSweepRead(const char *);
int RKSweepFree(RKSweep *);

#endif /* RKSweep_h */
