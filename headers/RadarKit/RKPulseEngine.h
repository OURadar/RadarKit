//
//  RKPulseEngine.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015-2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Pulse_Engine__
#define __RadarKit_Pulse_Engine__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct rk_pulse_worker RKPulseWorker;
typedef struct rk_pulse_engine RKPulseEngine;

typedef int RKPulseEnginePlanIndex[RKMaximumFilterCount];

struct rk_pulse_worker {
    RKShortName                      name;
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
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    uint32_t                         filterGroupCount;
    uint32_t                         filterCounts[RKMaximumFilterGroups];
    RKFilterAnchor                   filterAnchors[RKMaximumFilterGroups][RKMaximumFilterCount];
    RKComplex                        *filters[RKMaximumFilterGroups][RKMaximumFilterCount];
    void                             (*compressor)(RKCompressionScratch *);

    // Program set variables
    int                              *filterGid;
    RKPulseEnginePlanIndex      *planIndices;
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
int RKPulseEngineSetFilter(RKPulseEngine *,
                                const RKComplex *filter,
                                const RKFilterAnchor anchor,
                                const int group,
                                const int index);
int RKPulseEngineSetFilterToImpulse(RKPulseEngine *);
int RKPulseEngineSetFilterTo121(RKPulseEngine *);
int RKPulseEngineSetFilterTo11(RKPulseEngine *);

int RKPulseEngineStart(RKPulseEngine *);
int RKPulseEngineStop(RKPulseEngine *);

char *RKPulseEngineStatusString(RKPulseEngine *);
char *RKPulseEnginePulseString(RKPulseEngine *);
void RKPulseEngineFilterSummary(RKPulseEngine *);
void RKPulseEngineShowBuffer(fftwf_complex *, const int);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_Pulse_Engine__) */
