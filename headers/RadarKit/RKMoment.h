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
#include <RadarKit/RKPulsePair.h>
#include <RadarKit/RKMultiLag.h>

typedef int RKMomentEngineState;
enum RKMomentEngineState {
    RKMomentEngineStateNull,
    RKMomentEngineStateAllocated,
    RKMomentEngineStateActivating,
    RKMomentEngineStateActive,
    RKMomentEngineStateDeactivating,
    RKMomentEngineStateSleep
};

typedef struct rk_moment_worker RKMomentWorker;
typedef struct rk_moment_engine RKMomentEngine;

struct rk_moment_worker {
    RKMomentEngine         *parentEngine;
    char                   semaphoreName[16];
    pthread_t              tid;
    int                    id;
    uint32_t               tic;
    uint32_t               pid;
    double                 dutyBuff[RKWorkerDutyCycleBufferSize];
    double                 dutyCycle;                                // Latest duty cycle estimate
    float                  lag;                                      // Relative lag from the latest index
    sem_t                  *sem;
};

struct rk_moment_engine {
    // User set variables
    RKBuffer               pulseBuffer;
    uint32_t               *pulseIndex;
    uint32_t               pulseBufferSize;
    RKBuffer               rayBuffer;
    uint32_t               *rayIndex;
    uint32_t               rayBufferSize;
    uint8_t                verbose;
    uint8_t                coreCount;
    bool                   useSemaphore;
    bool                   developerMode;
    int                    (*processor)(RKScratch *, RKPulse **, const uint16_t, const char *);

    // Program set variables
    RKModuloPath           *momentSource;
    RKMomentWorker         *workers;
    pthread_t              tidPulseGatherer;
    pthread_mutex_t        coreMutex;
    uint8_t                processorLagCount;
    
    // Status / health
    RKMomentEngineState    state;
    uint32_t               tic;
    float                  lag;
    uint32_t               almostFull;
    size_t                 memoryUsage;
};

RKMomentEngine *RKMomentEngineInit(void);
void RKMomentEngineFree(RKMomentEngine *);

void RKMomentEngineSetVerbose(RKMomentEngine *, const int);
void RKMomentEngineSetDeveloperMode(RKMomentEngine *engine);
void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *,
                                         RKPulse *, uint32_t *, const uint32_t,
                                         RKRay *,   uint32_t *, const uint32_t);
void RKMomentEngineSetCoreCount(RKMomentEngine *, const int);
void RKMomentEngineSetMomentProcessorToMultilag(RKMomentEngine *engine);
void RKMomentEngineSetMomentProcessorToPulsePair(RKMomentEngine *engine);
void RKMomentEngineSetMomentProcessorToPulsePairHop(RKMomentEngine *engine);

int RKMomentEngineStart(RKMomentEngine *);
int RKMomentEngineStop(RKMomentEngine *);

char *RKMomentEngineStatusString(RKMomentEngine *engine);

#endif /* defined(___RadarKit_RKMoment__) */
