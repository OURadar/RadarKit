//
//  RKRawDataRecorder.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RawDataRecorder__
#define __RadarKit_RawDataRecorder__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>

#define RKRawDataRecorderDefaultMaximumRecorderDepth   100000
#define RKRawDataRecorderDefaultCacheSize              32 * 1024 * 1024

typedef struct rk_data_recorder RKRawDataRecorder;

struct rk_data_recorder {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKRawDataType                    recordType;
    RKBuffer                         pulseBuffer;                    // Buffer of raw pulses
    uint32_t                         *pulseIndex;                    // The refence index to watch for
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    uint8_t                          verbose;
    bool                             doNotWrite;
    size_t                           cacheSize;
    size_t                           maximumRecordDepth;
    RKFileManager                    *fileManager;

    // Program set variables
    int                              fd;
    FILE                             *fid;
    void                             *cache;
    size_t                           cacheWriteIndex;
    uint64_t                         cacheFlushCount;
    pthread_t                        tidPulseRecorder;

    // Status / health
    char                             statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t                         statusBufferIndex;
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    size_t                           memoryUsage;
};

RKRawDataRecorder *RKRawDataRecorderInit(void);
void RKRawDataRecorderFree(RKRawDataRecorder *engine);

void RKRawDataRecorderSetVerbose(RKRawDataRecorder *, const int);
void RKRawDataRecorderSetInputOutputBuffers(RKRawDataRecorder *engine, RKRadarDesc *, RKFileManager *,
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex);
void RKRawDataRecorderSetDoNotWrite(RKRawDataRecorder *engine, const bool value);
void RKRawDataRecorderSetMaximumRecordDepth(RKRawDataRecorder *engine, const uint32_t);
void RKRawDataRecorderSetCacheSize(RKRawDataRecorder *engine, uint32_t size);

int RKRawDataRecorderStart(RKRawDataRecorder *engine);
int RKRawDataRecorderStop(RKRawDataRecorder *engine);
char *RKRawDataRecorderStatusString(RKRawDataRecorder *engine);

size_t RKRawDataRecorderCacheWrite(RKRawDataRecorder *engine, const void *payload, const size_t size);
size_t RKRawDataRecorderCacheFlush(RKRawDataRecorder *engine);

#endif /* defined(__RadarKit_RKFile__) */
