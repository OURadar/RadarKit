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

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef struct rk_pulse_compression_engine {
    RKInt16Pulse    *input;
    RKFloatPulse    *output;
    uint32_t        *index;
    uint32_t        size;

    bool            active;

    unsigned int    coreCount;
    pthread_t       tidPulseWatcher;
    pthread_t       tid[256];
    uint32_t        tic[256];
    double          dutyCycle[256];

    bool            useSemaphore;
    char            semaphoreName[256][16];

    int             planCount;
    
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
//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKPulseCompression__) */
