//
//  RKPulseCompression.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015-2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKPulseCompression__
#define __RadarKit_RKPulseCompression__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>
#include <fftw3.h>

#define RKPulseCompressionDFTPlanCount   16

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct rk_pulse_compression_worker RKPulseCompressionWorker;
typedef struct rk_pulse_compression_engine RKPulseCompressionEngine;

typedef int RKPulseCompressionPlanIndex[RKPulseCompressionDFTPlanCount];

struct rk_pulse_compression_worker {
    int                        id;
    pthread_t                  tid;                                      // Thread ID
    uint32_t                   tic;                                      // Tic count
    uint32_t                   pid;                                      // Latest processed index of pulses buffer
    char                       semaphoreName[16];
    double                     dutyBuff[RKWorkerDutyCycleBufferDepth];
    double                     dutyCycle;                                // Latest duty cycle estimate
    float                      lag;                                      // Lag relative to the latest index of engine
    RKPulseCompressionEngine   *parentEngine;
};

struct rk_pulse_compression_engine {
    // User set variables
    char                             name[RKNameLength];
    RKBuffer                         pulseBuffer;                        // Buffer of raw pulses
    uint32_t                         *pulseIndex;                        // The refence index to watch for
    uint32_t                         pulseBufferDepth;                   // Size of the buffer
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    uint32_t                         configBufferDepth;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    bool                             useSemaphore;
    uint32_t                         filterGroupCount;
    uint32_t                         filterCounts[RKMaxFilterGroups];
    RKFilterAnchor                   filterAnchors[RKMaxFilterGroups][RKMaxFilterCount];
    RKComplex                        *filters[RKMaxFilterGroups][RKMaxFilterCount];

    // Program set variables
    int                              planCount;
    int                              planSizes[RKPulseCompressionDFTPlanCount];
    fftwf_plan                       planForwardInPlace[RKPulseCompressionDFTPlanCount];
    fftwf_plan                       planForwardOutPlace[RKPulseCompressionDFTPlanCount];
    fftwf_plan                       planBackwardInPlace[RKPulseCompressionDFTPlanCount];
    int                              *filterGid;
    RKPulseCompressionPlanIndex      *planIndices;
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

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void);
void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine);

void RKPulseCompressionEngineSetVerbose(RKPulseCompressionEngine *, const int);
void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine,
                                                   RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                                   RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth);
void RKPulseCompressionEngineSetCoreCount(RKPulseCompressionEngine *engine, const unsigned int count);

int RKPulseCompressionResetFilters(RKPulseCompressionEngine *engine);
int RKPulseCompressionSetFilterCountOfGroup(RKPulseCompressionEngine *engine, const int group, const int count);
int RKPulseCompressionSetFilterGroupCount(RKPulseCompressionEngine *engine, const int groupCount);
int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine,
                                const RKComplex *filter,
                                const RKFilterAnchor anchor,
                                const int group,
                                const int index);
int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine);
int RKPulseCompressionSetFilterTo121(RKPulseCompressionEngine *engine);
int RKPulseCompressionSetFilterTo11(RKPulseCompressionEngine *engine);

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine);
int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine);
char *RKPulseCompressionEngineStatusString(RKPulseCompressionEngine *engine);
void RKPulseCompressionFilterSummary(RKPulseCompressionEngine *engine);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKPulseCompression__) */
