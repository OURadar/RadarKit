//
//  RKSweep.h
//  RadarKit
//
//  Created by Boonleng Cheong on 1/15/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Sweep__
#define __RadarKit_Sweep__

#define RKSweepScratchSpaceDepth             6
#define RKMaximumProductBufferDepth          20
#define RKMaximumListLength                  RKMaximumPathLength + RKMaximumProductBufferDepth * RKMaximumPathLength

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>
#include <RadarKit/RKSweepFile.h>
#include <RadarKit/RKProduct.h>
#include <RadarKit/RKProductFile.h>

typedef struct rk_sweep_scratch {
    char                             filelist[RKMaximumListLength];              // It's really handleFilesScript + file list
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
    uint8_t                          verbose;
    bool                             record;
    bool                             hasFileHandlingScript;
    char                             fileHandlingScript[RKMaximumPathLength];
    RKScriptProperty                 fileHandlingScriptProperties;
    RKFileManager                    *fileManager;
    char                             productFileExtension[RKMaximumFileExtensionLength];
    int                              (*productRecorder)(RKProduct *, const char *);
    int                              (*productCollectionRecorder)(RKProductCollection *, const char *, uint32_t);

    // Program set variables
    RKIdentifier                     sweepIndex;
    pthread_t                        tidRayGatherer;
    RKSweepScratchSpace              scratchSpaces[RKSweepScratchSpaceDepth];
    uint8_t                          scratchSpaceIndex;
    uint8_t                          lastRecordedScratchSpaceIndex;
    RKProduct                        *productBuffer;
    uint32_t                         productBufferDepth;
    uint32_t                         productIndex;
    pthread_mutex_t                  productMutex;
    RKProductList                    productList;
    RKProductId                      productIds[RKProductIndexCount];
    uint32_t                         business;

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
void RKSweepEngineSetEssentials(RKSweepEngine *, RKRadarDesc *, RKFileManager *,
                                RKConfig *configBuffer,   uint32_t *configIndex,
                                RKBuffer rayBuffer,       uint32_t *rayIndex);
void RKSweepEngineSetRecord(RKSweepEngine *, const bool);
void RKSweepEngineSetProductTimeout(RKSweepEngine *, const uint32_t);
void RKSweepEngineSetFilesHandlingScript(RKSweepEngine *, const char *, const RKScriptProperty);
void RKSweepEngineSetProductRecorder(RKSweepEngine *, int (*)(RKProduct *, const char *));

int RKSweepEngineStart(RKSweepEngine *);
int RKSweepEngineStop(RKSweepEngine *);
void RKSweepEngineFlush(RKSweepEngine *);

char *RKSweepEngineStatusString(RKSweepEngine *);
char *RKSweepEngineLatestSummary(RKSweepEngine *);

RKProductId RKSweepEngineDescribeProduct(RKSweepEngine *, RKProductDesc);
int RKSweepEngineUndescribeProduct(RKSweepEngine *, RKProductId);
RKProduct *RKSweepEngineGetVacantProduct(RKSweepEngine *, RKSweep *, RKProductId);
int RKSweepEngineSetProductComplete(RKSweepEngine *, RKSweep *, RKProduct *);
void RKSweepEngineWaitWhileBusy(RKSweepEngine *);

RKSweep *RKSweepCollect(RKSweepEngine *, const uint8_t);
int RKSweepFree(RKSweep *);

#endif /* RKSweep_h */
