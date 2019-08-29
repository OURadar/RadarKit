//
//  RKSweep.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Sweep__
#define __RadarKit_Sweep__

#define RKSweepScratchSpaceDepth 4

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>
#include <RadarKit/RKSweepFile.h>
#include <RadarKit/RKProduct.h>
#include <RadarKit/RKProductFile.h>

typedef struct rk_sweep_scratch {
    char                             filename[RKMaximumPathLength - 32];
    char                             filelist[RKMaximumStringLength];              // It's really handleFilesScript + file list
    char                             summary[RKMaximumStringLength];
    RKRay                            *rays[RKMaximumRaysPerSweep];
    uint32_t                         rayCount;
} RKSweepScratchSpace;

typedef struct rk_sweep_engine RKSweepEngine;

struct rk_sweep_engine {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKBuffer                         rayBuffer;
    uint32_t                         *rayIndex;
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    RKProduct                        *productBuffer;
    uint32_t                         *productIndex;
    uint8_t                          verbose;
    bool                             record;
    bool                             convertToDegrees;
    bool                             hasFileHandlingScript;
    char                             fileHandlingScript[RKMaximumPathLength];
    RKScriptProperty                 fileHandlingScriptProperties;
    RKFileManager                    *fileManager;
    uint32_t                         productTimeoutSeconds;
    char                             productFileExtension[RKMaximumFileExtensionLength];
    int                              (*productRecorder)(RKProduct *, const char *);

    // Program set variables
    pthread_t                        tidRayGatherer;
    RKSweepScratchSpace              scratchSpaces[RKSweepScratchSpaceDepth];
    uint8_t                          scratchSpaceIndex;
    pthread_mutex_t                  productMutex;
    RKBaseMomentList                 baseMomentList;
    RKProductId                      baseMomentProductIds[RKBaseMomentIndexCount];

    // Status / health
    uint32_t                         processedRayIndex;
    char                             statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
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
                                       RKConfig *configBuffer,   uint32_t *configIndex,
                                       RKBuffer rayBuffer,       uint32_t *rayIndex,
                                       RKProduct *productBuffer, uint32_t *productIndex);
void RKSweepEngineSetRecord(RKSweepEngine *, const bool);
void RKSweepEngineSetProductTimeout(RKSweepEngine *, const uint32_t);
void RKSweepEngineSetFilesHandlingScript(RKSweepEngine *, const char *, const RKScriptProperty);
void RKSweepEngineSetProductRecorder(RKSweepEngine *, int (*)(RKProduct *, const char *));

int RKSweepEngineStart(RKSweepEngine *);
int RKSweepEngineStop(RKSweepEngine *);
char *RKSweepEngineStatusString(RKSweepEngine *);

RKProductId RKSweepEngineRegisterProduct(RKSweepEngine *, RKProductDesc);
int RKSweepEngineUnregisterProduct(RKSweepEngine *, RKProductId);
RKProduct *RKSweepEngineGetVacantProduct(RKSweepEngine *, RKSweep *, RKProductId);
int RKSweepEngineSetProductComplete(RKSweepEngine *, RKSweep *, RKProduct *);

RKSweep *RKSweepCollect(RKSweepEngine *, const uint8_t);
int RKSweepFree(RKSweep *);

#endif /* RKSweep_h */
