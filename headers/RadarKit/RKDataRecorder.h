//
//  RKDataRecorder.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_DataRecorder__
#define __RadarKit_DataRecorder__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>

typedef struct rk_data_recorder RKDataRecorder;

struct rk_data_recorder {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    RKBuffer               pulseBuffer;                    // Buffer of raw pulses
    uint32_t               *pulseIndex;                    // The refence index to watch for
    uint32_t               pulseBufferDepth;               // Size of the buffer
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint32_t               configBufferDepth;
    uint8_t                verbose;
    bool                   doNotWrite;
    uint32_t               cacheSize;
    RKFileManager          *fileManager;

    // Program set variables
    int                    fd;
    FILE                   *fid;
    void                   *cache;
    uint32_t               cacheWriteIndex;
    pthread_t              tidPulseRecorder;

    // Status / health
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    uint32_t               tic;
    float                  lag;
    size_t                 memoryUsage;
};

RKDataRecorder *RKDataRecorderInit(void);
void RKDataRecorderFree(RKDataRecorder *engine);

void RKDataRecorderSetVerbose(RKDataRecorder *, const int);
void RKDataRecorderSetInputOutputBuffers(RKDataRecorder *engine, RKRadarDesc *, RKFileManager *,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth);
void RKDataRecorderSetDoNotWrite(RKDataRecorder *engine, const bool value);
int RKDataRecorderStart(RKDataRecorder *engine);
int RKDataRecorderStop(RKDataRecorder *engine);
char *RKDataRecorderStatusString(RKDataRecorder *engine);

void RKDataRecorderSetCacheSize(RKDataRecorder *engine, uint32_t size);
uint32_t RKDataRecorderCacheWrite(RKDataRecorder *engine, const void *payload, const uint32_t size);
uint32_t RKDataRecorderCacheFlush(RKDataRecorder *engine);

#endif /* defined(__RadarKit_RKFile__) */
