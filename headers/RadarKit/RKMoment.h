//
//  RKMoment.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Moment__
#define __RadarKit_Moment__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>
#include <RadarKit/RKPulsePair.h>
#include <RadarKit/RKMultiLag.h>

typedef struct rk_moment_worker RKMomentWorker;
typedef struct rk_moment_engine RKMomentEngine;

struct rk_moment_worker {
    RKMomentEngine                   *parentEngine;
    char                             semaphoreName[32];
    int                              id;
    pthread_t                        tid;
    uint64_t                         tic;
    uint32_t                         pid;
    double                           dutyBuff[RKWorkerDutyCycleBufferDepth];
    double                           dutyCycle;                                // Latest duty cycle estimate
    float                            lag;                                      // Relative lag from the latest index
    sem_t                            *sem;
};

struct rk_moment_engine {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    RKBuffer                         pulseBuffer;
    uint32_t                         *pulseIndex;
    RKBuffer                         rayBuffer;
    uint32_t                         *rayIndex;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    int                              (*processor)(RKScratch *, RKPulse **, const uint16_t pulseCount);

    // Program set variables
    RKModuloPath                     *momentSource;
    RKMomentWorker                   *workers;
    pthread_t                        tidPulseGatherer;
    pthread_mutex_t                  coreMutex;
    uint8_t                          processorLagCount;
    uint8_t                          userLagChoice;
    
    // Status / health
    uint32_t                         processedPulseIndex;
    char                             statusBuffer[RKBufferSSlotCount][RKNameLength];
    char                             rayStatusBuffer[RKBufferSSlotCount][RKNameLength];
    uint32_t                         statusBufferIndex;
    uint32_t                         rayStatusBufferIndex;
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    uint32_t                         almostFull;
    size_t                           memoryUsage;
};

RKMomentEngine *RKMomentEngineInit(void);
void RKMomentEngineFree(RKMomentEngine *);

void RKMomentEngineSetVerbose(RKMomentEngine *, const int verbose);
void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *, const RKRadarDesc *,
                                         RKConfig *configBuffer, uint32_t *configIndex,
                                         RKBuffer pulseBuffer, uint32_t *pulseIndex,
                                         RKBuffer rayBuffer,   uint32_t *rayIndex);
void RKMomentEngineSetCoreCount(RKMomentEngine *, const uint8_t);
void RKMomentEngineSetCoreOrigin(RKMomentEngine *, const uint8_t);

int RKMomentEngineStart(RKMomentEngine *);
int RKMomentEngineStop(RKMomentEngine *);

char *RKMomentEngineStatusString(RKMomentEngine *);

#endif /* defined(___RadarKit_RKMoment__) */
