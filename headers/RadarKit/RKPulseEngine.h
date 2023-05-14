//
//  RKPulseEngine.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/18/15.
//  Copyright (c) 2015-2017 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Pulse_Engine__
#define __RadarKit_Pulse_Engine__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKScratch.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKDSP.h>

typedef struct rk_pulse_worker RKPulseWorker;
typedef struct rk_pulse_engine RKPulseEngine;

typedef int RKPulseEnginePlanIndex[RKMaximumFilterCount];

struct rk_pulse_worker {
    RKName                           name;
    int                              id;
    pthread_t                        tid;                                      // Thread ID
    RKPulseEngine                    *parent;                                  // Parent engine reference

    char                             semaphoreName[32];
    uint64_t                         tic;                                      // Tic count
    uint32_t                         pid;                                      // Latest processed index of pulses buffer
    double                           dutyBuff[RKWorkerDutyCycleBufferDepth];   // Duty cycle history
    double                           dutyCycle;                                // Latest duty cycle estimate
    float                            lag;                                      // Relative lag from the latest index
    sem_t                            *sem;

    RKCompressor                     compressor;
};

struct rk_pulse_engine {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    RKBuffer                         pulseBuffer;                              // Buffer of raw pulses
    uint32_t                         *pulseIndex;                              // The refence index to watch for
    RKFFTModule                      *fftModule;
    RKUserModule                     userModule;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    uint32_t                         filterGroupCount;
    uint32_t                         filterCounts[RKMaximumWaveformCount];
    RKFilterAnchor                   filterAnchors[RKMaximumWaveformCount][RKMaximumFilterCount];
    RKComplex                        *filters[RKMaximumWaveformCount][RKMaximumFilterCount];
    // void                             (*configChangeCallback)(RKCompressionScratch *);
    void                             (*compressor)(RKUserModule, RKCompressionScratch *);

    // Program set variables
    int                              *filterGid;
    RKPulseEnginePlanIndex           *planIndices;
    RKPulseWorker                    *workers;
    pthread_t                        tidPulseWatcher;
    pthread_mutex_t                  mutex;

    // Status / health
    char                             statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    char                             pulseStatusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t                         statusBufferIndex;
    uint32_t                         pulseStatusBufferIndex;
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    float                            minWorkerLag;
    float                            maxWorkerLag;
    int                              almostFull;
    size_t                           memoryUsage;
};

RKPulseEngine *RKPulseEngineInit(void);
void RKPulseEngineFree(RKPulseEngine *);

void RKPulseEngineSetVerbose(RKPulseEngine *, const int);
void RKPulseEngineSetInputOutputBuffers(RKPulseEngine *, const RKRadarDesc *,
                                        RKConfig *configBuffer, uint32_t *configIndex,
                                        RKBuffer pulseBuffer,   uint32_t *pulseIndex);
void RKPulseEngineSetFFTModule(RKPulseEngine *, RKFFTModule *);
void RKPulseEngineSetCoreCount(RKPulseEngine *, const uint8_t);
void RKPulseEngineSetCoreOrigin(RKPulseEngine *, const uint8_t);

int RKPulseEngineResetFilters(RKPulseEngine *);
int RKPulseEngineSetFilterCountOfGroup(RKPulseEngine *, const int group, const int count);
int RKPulseEngineSetFilterGroupCount(RKPulseEngine *, const int groupCount);
int RKPulseEngineSetGroupFilter(RKPulseEngine *,
                                const RKComplex *filter,
                                const RKFilterAnchor anchor,
                                const int group,
                                const int index);
int RKPulseEngineSetFilter(RKPulseEngine *, const RKComplex *, const RKFilterAnchor anchor);
int RKPulseEngineSetFilterByWaveform(RKPulseEngine *, RKWaveform *);
int RKPulseEngineSetFilterToImpulse(RKPulseEngine *);

int RKPulseEngineStart(RKPulseEngine *);
int RKPulseEngineStop(RKPulseEngine *);

void RKPulseEngineWaitWhileBusy(RKPulseEngine *);

char *RKPulseEngineStatusString(RKPulseEngine *);
char *RKPulseEnginePulseString(RKPulseEngine *);
void RKPulseEngineFilterSummary(RKPulseEngine *);

void RKBuiltInCompressor(RKUserModule, RKCompressionScratch *);

#endif /* defined(__RadarKit_Pulse_Engine__) */
