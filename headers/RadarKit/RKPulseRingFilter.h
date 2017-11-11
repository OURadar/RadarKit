//
//  RKPulseRingFilter.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 11/11/17.
//  Copyright (c) 2015-2018 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_PulseRingFilter__
#define __RadarKit_PulseRingFilter__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/DSP.h>

typedef struct rk_pulse_ring_filter_worker RKPulseRingFilterWorker;
typedef struct rk_pulse_ring_filter_engine RKPulseRingFilterEngine;

struct rk_pulse_ring_filter_worker {
    RKPulseRingFilterEngine   *parentEngine;
    char                      semaphoreName[16];
    int                       id;
    pthread_t                  tid;                                      // Thread ID
    uint32_t                   tic;                                      // Tic count
    uint32_t                   pid;                                      // Latest processed index of pulses buffer
    double                     dutyBuff[RKWorkerDutyCycleBufferDepth];
    double                     dutyCycle;                                // Latest duty cycle estimate
    float                      lag;                                      // Lag relative to the latest index of engine
    sem_t                      *sem;
};

struct rk_pulse_ring_filter_engine {
    // User set variables
    char                             name[RKNameLength];
    RKRadarDesc                      *radarDescription;
    RKBuffer                         pulseBuffer;                        // Buffer of raw pulses
    uint32_t                         *pulseIndex;                        // The refence index to watch for
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    RKComplex                        *filters[2];
    
    // Program set variables
    RKPulseCompressionWorker         *workers;
    pthread_t                        tidPulseWatcher;
    pthread_mutex_t                  coreMutex;
    
    // Status / health
    char                             statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t                         statusBufferIndex;
    RKEngineState                    state;
    uint32_t                         tic;
    float                            lag;
    int                              almostFull;
    size_t                           memoryUsage;
};

RKPulseRingFilterEngine *RKPulseRingFilterEngineInit(void);
void RKPulseRingFilterEngineFree(RKPulseRingFilterEngine *);

RKPulseRingFilterEngineStop(RKPulseRingFilterEngine *);

#endif
