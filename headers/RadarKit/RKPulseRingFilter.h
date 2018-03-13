//
//  RKPulseRingFilter.h
//  RadarKit
//
//  NOTE: This module was started for FIR/IIR filters but it would
//  probably never be used so this engine is mostly doing nothing.
//
//  Created by Boon Leng Cheong on 11/11/17.
//  Copyright (c) 2015-2018 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_PulseRingFilter__
#define __RadarKit_PulseRingFilter__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

typedef struct rk_pulse_ring_filter_worker RKPulseRingFilterWorker;
typedef struct rk_pulse_ring_filter_engine RKPulseRingFilterEngine;

struct rk_pulse_ring_filter_worker {
    RKPulseRingFilterEngine    *parentEngine;
    char                       semaphoreName[16];
    int                        id;
    pthread_t                  tid;                                      // Thread ID
    uint32_t                   tic;                                      // Tic count
    uint32_t                   pid;                                      // Latest processed index of pulses buffer
    double                     dutyBuff[RKWorkerDutyCycleBufferDepth];   // Duty cycle history
    double                     dutyCycle;                                // Latest duty cycle estimate
    float                      lag;                                      // Lag relative to the latest index of engine
    sem_t                      *sem;
};

struct rk_pulse_ring_filter_engine {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKBuffer                         pulseBuffer;                        // Buffer of raw pulses
    uint32_t                         *pulseIndex;                        // The refence index to watch for
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    RKComplex                        *filters[2];                        // Coefficients b & a
    RKModuloPath                     *filterLinePath;                    // The origin and length for each worker
    bool                             *workerTaskDone;                    // Task status [coreCount x pulseBufferDepth]
    
    // Program set variables
    RKPulseRingFilterWorker          *workers;
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

void RKPulseRingFilterEngineSetVerbose(RKPulseRingFilterEngine *, const int);
void RKPulseRingFilterEngineSetInputOutputBuffers(RKPulseRingFilterEngine *, const RKRadarDesc *,
                                                  RKConfig *configBuffer, uint32_t *configIndex,
                                                  RKBuffer pulseBuffer,   uint32_t *pulseIndex);
void RKPulseRingFilterEngineSetCoreCount(RKPulseRingFilterEngine *, const uint8_t);
void RKPulseRingFilterEngineSetCoreOrigin(RKPulseRingFilterEngine *, const uint8_t);

int RKPulseRingFilterEngineSetFilter(RKPulseRingFilterEngine *, RKIIRFilter *);

int RKPulseRingFilterEngineStart(RKPulseRingFilterEngine *);
int RKPulseRingFilterEngineStop(RKPulseRingFilterEngine *);

char *RKPulseRingFilterEngineStatusString(RKPulseRingFilterEngine *);

#endif
