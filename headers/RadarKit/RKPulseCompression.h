//
//  RKPulseCompression.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015-2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_PulseCompression__
#define __RadarKit_PulseCompression__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct rk_pulse_compression_worker RKPulseCompressionWorker;
typedef struct rk_pulse_compression_engine RKPulseCompressionEngine;

typedef int RKPulseCompressionPlanIndex[RKMaximumFilterCount];

struct rk_pulse_compression_worker {
    RKShortName                      name;
    int                              id;
    pthread_t                        tid;                                      // Thread ID
    RKPulseCompressionEngine         *parent;                                  // Parent engine reference

    char                             semaphoreName[32];
    uint64_t                         tic;                                      // Tic count
    uint32_t                         pid;                                      // Latest processed index of pulses buffer
    double                           dutyBuff[RKWorkerDutyCycleBufferDepth];   // Duty cycle history
    double                           dutyCycle;                                // Latest duty cycle estimate
    float                            lag;                                      // Relative lag from the latest index
    sem_t                            *sem;
};

struct rk_pulse_compression_engine {
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
    RKPulseCompressionPlanIndex      *planIndices;
    RKPulseCompressionWorker         *workers;
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

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void);
void RKPulseCompressionEngineFree(RKPulseCompressionEngine *);

void RKPulseCompressionEngineSetVerbose(RKPulseCompressionEngine *, const int);
void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *, const RKRadarDesc *,
                                                   RKConfig *configBuffer, uint32_t *configIndex,
                                                   RKBuffer pulseBuffer,   uint32_t *pulseIndex);
void RKPulseCompressionEngineSetFFTModule(RKPulseCompressionEngine *, RKFFTModule *);
void RKPulseCompressionEngineSetCoreCount(RKPulseCompressionEngine *, const uint8_t);
void RKPulseCompressionEngineSetCoreOrigin(RKPulseCompressionEngine *, const uint8_t);

int RKPulseCompressionResetFilters(RKPulseCompressionEngine *);
int RKPulseCompressionSetFilterCountOfGroup(RKPulseCompressionEngine *, const int group, const int count);
int RKPulseCompressionSetFilterGroupCount(RKPulseCompressionEngine *, const int groupCount);
int RKPulseCompressionSetFilter(RKPulseCompressionEngine *,
                                const RKComplex *filter,
                                const RKFilterAnchor anchor,
                                const int group,
                                const int index);
int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *);
int RKPulseCompressionSetFilterTo121(RKPulseCompressionEngine *);
int RKPulseCompressionSetFilterTo11(RKPulseCompressionEngine *);

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *);
int RKPulseCompressionEngineStop(RKPulseCompressionEngine *);

char *RKPulseCompressionEngineStatusString(RKPulseCompressionEngine *);
char *RKPulseCompressionEnginePulseString(RKPulseCompressionEngine *);
void RKPulseCompressionFilterSummary(RKPulseCompressionEngine *);
void RKPulseCompressionShowBuffer(fftwf_complex *, const int);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKPulseCompression__) */
