//
//  RKMoment.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKMoment__
#define __RadarKit_RKMoment__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>
#include <RadarKit/RKPulsePair.h>
#include <RadarKit/RKMultiLag.h>

typedef struct rk_moment_worker RKMomentWorker;
typedef struct rk_moment_engine RKMomentEngine;

struct rk_moment_worker {
    RKMomentEngine         *parentEngine;
    char                   semaphoreName[16];
    int                    id;
    pthread_t              tid;
    uint32_t               tic;
    uint32_t               pid;
    double                 dutyBuff[RKWorkerDutyCycleBufferDepth];
    double                 dutyCycle;                                // Latest duty cycle estimate
    float                  lag;                                      // Relative lag from the latest index
    sem_t                  *sem;
};

struct rk_moment_engine {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint32_t               configBufferDepth;
    RKBuffer               pulseBuffer;
    uint32_t               *pulseIndex;
    uint32_t               pulseBufferDepth;
    RKBuffer               rayBuffer;
    uint32_t               *rayIndex;
    uint32_t               rayBufferDepth;
    uint8_t                verbose;
    uint8_t                coreCount;
    bool                   useSemaphore;
    int                    (*processor)(RKScratch *, RKPulse **, const uint16_t pulseCount);

    // Program set variables
    RKModuloPath           *momentSource;
    RKMomentWorker         *workers;
    pthread_t              tidPulseGatherer;
    pthread_mutex_t        coreMutex;
    uint8_t                processorLagCount;
    
    // Status / health
    uint32_t               processedPulseIndex;
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    char                   rayStatusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    uint32_t               rayStatusBufferIndex;
    RKEngineState          state;
    uint32_t               tic;
    float                  lag;
    uint32_t               almostFull;
    size_t                 memoryUsage;
};

RKMomentEngine *RKMomentEngineInit(void);
void RKMomentEngineFree(RKMomentEngine *);

void RKMomentEngineSetVerbose(RKMomentEngine *, const int verbose);
void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *, RKRadarDesc *,
                                         RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                         RKBuffer pulseBuffer, uint32_t *pulseIndex, const uint32_t pulseBufferDepth,
                                         RKBuffer rayBuffer,   uint32_t *rayIndex,   const uint32_t rayBufferDepth);
void RKMomentEngineSetCoreCount(RKMomentEngine *, const int);
void RKMomentEngineSetMomentProcessorToMultilag(RKMomentEngine *);
void RKMomentEngineSetMomentProcessorToPulsePair(RKMomentEngine *);
void RKMomentEngineSetMomentProcessorToPulsePairHop(RKMomentEngine *);

int RKMomentEngineStart(RKMomentEngine *);
int RKMomentEngineStop(RKMomentEngine *);

char *RKMomentEngineStatusString(RKMomentEngine *);

#endif /* defined(___RadarKit_RKMoment__) */
