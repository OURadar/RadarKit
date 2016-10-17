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

#define RKPulseCompressionDFTPlanCount   4
#define RKMaxMatchedFilterCount          3   // Maximum filter count within each group
#define RKMaxMatchedFilterGroupCount     8   // Maximum filter group

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct rk_filter_rect {
    int origin;
    int length;
} RKPulseCompressionFilterAnchor;

typedef struct rk_pulse_compression_worker {
    int         planCount;
    int         planSizes[RKPulseCompressionDFTPlanCount];
    fftwf_plan  planInForward[RKPulseCompressionDFTPlanCount];
    fftwf_plan  planOutBackward[RKPulseCompressionDFTPlanCount];
    fftwf_plan  planFilterForward[RKMaxMatchedFilterGroupCount][RKMaxMatchedFilterCount][RKPulseCompressionDFTPlanCount];
} RKPulseCompressionWorker;

typedef struct rk_pulse_compression_engine {
    RKInt16Pulse                     *input;
    RKFloatPulse                     *output;
    uint32_t                         *index;
    uint32_t                         size;

    bool                             active;

    unsigned int                     coreCount;
    pthread_t                        tidPulseWatcher;
    pthread_t                        tid[256];
    uint32_t                         tic[256];
    double                           dutyCycle[256];

    bool                             useSemaphore;
    char                             semaphoreName[256][16];

    uint32_t                         filterGroupCount;
    uint32_t                         filterCounts[RKMaxMatchedFilterCount];
    uint32_t                         filterLengths[RKMaxMatchedFilterCount];
    RKComplex                        *filters[RKMaxMatchedFilterGroupCount][RKMaxMatchedFilterCount];
    RKPulseCompressionFilterAnchor   anchors[RKMaxMatchedFilterGroupCount][RKMaxMatchedFilterCount];
    RKPulseCompressionWorker         *workers;

    pthread_mutex_t                  coreMutex;
} RKPulseCompressionEngine;

RKPulseCompressionEngine *RKPulseCompressionEngineInitWithCoreCount(const unsigned int count);
RKPulseCompressionEngine *RKPulseCompressionEngineInit(void);
int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine);
int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine);
void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine);
void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine,
                                                   RKInt16Pulse *input,
                                                   RKFloatPulse *output,
                                                   uint32_t *index,
                                                   const uint32_t size);
int RKPulseCompressionSetFilterCountOfGroup(RKPulseCompressionEngine *engine, const int group, const int count);
int RKPulseCompressionSetFilterGroupCount(RKPulseCompressionEngine *engine, const int groupCount);
int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine,
                                const RKComplex *filter,
                                const int filterLength,
                                const int dataOrigin,
                                const int dataLength,
                                const int group,
                                const int index);
int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKPulseCompression__) */
