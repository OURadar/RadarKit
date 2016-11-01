//
//  RKMoment.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//
//

#ifndef __RadarKit_RKMoment__
#define __RadarKit_RKMoment__

#include <RadarKit/RKFoundation.h>

typedef int RKMomentEngineState;
enum RKMomentEngineState {
    RKMomentEngineStateNull,
    RKMomentEngineStateAllocated,
    RKMomentEngineStateActivating,
    RKMomentEngineStateActive,
    RKMomentEngineStateDeactivating,
    RKMomentEngineStateSleep
};

typedef struct rk_moment_source RKMomentSource;
typedef struct rk_moment_worker RKMomentWorker;
typedef struct rk_moment_engine RKMomentEngine;

struct rk_moment_source {
    uint32_t origin;
    uint32_t length;
};

struct rk_moment_worker {
    int                    id;
    pthread_t              tid;
    uint32_t               tic;
    uint32_t               pid;
    char                   semaphoreName[16];
    double                 dutyBuff[RKWorkerDutyCycleBufferSize];
    double                 dutyCycle;                                // Latest duty cycle estimate
    float                  lag;                                      // Lag relative to the latest index of engine
    RKMomentEngine         *parentEngine;
};

struct rk_moment_engine {
    RKPulse                *pulses;
    uint32_t               *pulseIndex;
    uint32_t               pulseBufferSize;
    RKFloatRay             *rays;
    RKInt16Ray             *encodedRays;
    uint32_t               *rayIndex;
    uint32_t               rayBufferSize;
    uint32_t               tic;
    uint32_t               verbose;
    uint32_t               coreCount;
    bool                   useSemaphore;
    int                    (*processor)(RKMomentEngine *, const int, char *);

    RKMomentSource         *momentSource;
    RKMomentWorker         *workers;

    RKMomentEngineState    state;
    uint32_t               almostFull;
    pthread_t              tidPulseGatherer;
    pthread_mutex_t        coreMutex;
};

RKMomentEngine *RKMomentEngineInit(void);
void RKMomentEngineFree(RKMomentEngine *engine);

void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *engine,
                                         RKPulse *pulses,
                                         uint32_t *pulseIndex,
                                         const uint32_t pulseBufferSize,
                                         RKFloatRay *rays,
                                         uint32_t *rayIndex,
                                         const uint32_t rayBufferSize);
void RKMomentEngineSetCoreCount(RKMomentEngine *engine, const unsigned int count);

int RKMomentEngineStart(RKMomentEngine *engine);
int RKMomentEngineStop(RKMomentEngine *engine);

char *RKMomentEngineStatusString(RKMomentEngine *engine);

#endif /* defined(___RadarKit_RKMoment__) */
