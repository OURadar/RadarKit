//
//  RKPulseCompression.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKPulseCompression__
#define __RadarKit_RKPulseCompression__

#include <RadarKit/RKFoundation.h>
#include <fftw3.h>

#define RKPulseCompressionDFTPlanCount   16
#define RKWorkerDutyCycleBufferSize      1000

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef int RKPulseCompressionEngineState;
enum RKPulseCompressionEngineState {
    RKPulseCompressionEngineStateNull,
    RKPulseCompressionEngineStateAllocated,
    RKPulseCompressionEngineStateActivating,
    RKPulseCompressionEngineStateActive,
    RKPulseCompressionEngineStateDeactivating,
    RKPulseCompressionEngineStateSleep
};

typedef struct rk_filter_rect {
    int origin;
    int length;
    int maxDataLength;
} RKPulseCompressionFilterAnchor;

typedef struct rk_pulse_compression_worker RKPulseCompressionWorker;
typedef struct rk_pulse_compression_engine RKPulseCompressionEngine;

struct rk_pulse_compression_worker {
    int                        id;
    uint32_t                   tic;
    pthread_t                  tid;                                      // Thread ID
    uint32_t                   pid;                                      // Latest processed index of pulses buffer
    char                       semaphoreName[16];
    double                     dutyBuff[RKWorkerDutyCycleBufferSize];
    double                     dutyCycle;                                // Latest duty cycle estimate
    float                      lag;                                      // Lag relative to the latest index of engine
    RKPulseCompressionEngine   *parentEngine;
};

typedef int RKPulseCompressionPlanIndex[RKPulseCompressionDFTPlanCount];

struct rk_pulse_compression_engine {
    RKPulse                          *pulses;
    uint32_t                         *index;
    uint32_t                         size;
    uint32_t                         tic;              // Process count

    RKPulseCompressionEngineState    state;
    int                              verbose;

    unsigned int                     coreCount;
    pthread_t                        tidPulseWatcher;

    bool                             useSemaphore;

    uint32_t                         filterGroupCount;
    uint32_t                         filterCounts[RKMaxMatchedFilterGroupCount];
    RKComplex                        *filters[RKMaxMatchedFilterGroupCount][RKMaxMatchedFilterCount];
    RKPulseCompressionFilterAnchor   anchors[RKMaxMatchedFilterGroupCount][RKMaxMatchedFilterCount];

    int                              planCount;
    int                              planSizes[RKPulseCompressionDFTPlanCount];
    fftwf_plan                       planForwardInPlace[RKPulseCompressionDFTPlanCount];
    fftwf_plan                       planForwardOutPlace[RKPulseCompressionDFTPlanCount];
    fftwf_plan                       planBackwardInPlace[RKPulseCompressionDFTPlanCount];

    int                              *filterGid;
    RKPulseCompressionPlanIndex      *planIndices;

    RKPulseCompressionWorker         *workers;

    int                              almostFull;
    
    pthread_mutex_t                  coreMutex;
};

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void);
void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine);

void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine,
                                                   RKPulse *pulses,
                                                   uint32_t *index,
                                                   const uint32_t size);
void RKPulseCompressionEngineSetCoreCount(RKPulseCompressionEngine *engine, const unsigned int count);
int RKPulseCompressionSetFilterCountOfGroup(RKPulseCompressionEngine *engine, const int group, const int count);
int RKPulseCompressionSetFilterGroupCount(RKPulseCompressionEngine *engine, const int groupCount);
int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine,
                                const RKComplex *filter,
                                const int filterLength,
                                const int dataOrigin,
                                const int maxDataLength,
                                const int group,
                                const int index);
int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine);
int RKPulseCompressionSetFilterTo121(RKPulseCompressionEngine *engine);

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine);
int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine);

void RKPulseCompressionEngineLogStatus(RKPulseCompressionEngine *engine);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKPulseCompression__) */
