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

#define RKDataRecorderDefaultMaximumRecorderDepth   100000
#define RKDataRecorderDefaultCacheSize              32 * 1024 * 1024

typedef struct rk_data_recorder RKDataRecorder;

struct rk_data_recorder {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    RKBuffer               pulseBuffer;                    // Buffer of raw pulses
    uint32_t               *pulseIndex;                    // The refence index to watch for
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint8_t                verbose;
    bool                   doNotWrite;
    size_t                 cacheSize;
    size_t                 maximumRecordDepth;
    RKFileManager          *fileManager;

    // Program set variables
    int                    fd;
    FILE                   *fid;
    void                   *cache;
    size_t                 cacheWriteIndex;
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
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex);
void RKDataRecorderSetDoNotWrite(RKDataRecorder *engine, const bool value);
void RKDataRecorderSetMaximumRecordDepth(RKDataRecorder *engine, const uint32_t);
void RKDataRecorderSetCacheSize(RKDataRecorder *engine, uint32_t size);

int RKDataRecorderStart(RKDataRecorder *engine);
int RKDataRecorderStop(RKDataRecorder *engine);
char *RKDataRecorderStatusString(RKDataRecorder *engine);

size_t RKDataRecorderCacheWrite(RKDataRecorder *engine, const void *payload, const size_t size);
size_t RKDataRecorderCacheFlush(RKDataRecorder *engine);

#endif /* defined(__RadarKit_RKFile__) */
