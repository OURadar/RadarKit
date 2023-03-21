
//
//  RKPositionSteerEngine.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_PositionSteerEngine__
#define __RadarKit_PositionSteerEngine__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKDSP.h>

typedef struct rk_position_steer_engine RKPositionSteerEngine;

struct rk_position_steer_engine {
    // User set variables
    RKName                 name;
    RKRadarDesc            *radarDescription;
    RKPosition             *positionBuffer;
    uint32_t               *positionIndex;
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint8_t                verbose;

    // Program set variables
    pthread_t              threadId;
    int                    vcpIndex;
    int                    vcpSweepCount;

    // Status / health
    size_t                 memoryUsage;
    char                   statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    uint64_t               tic;
    float                  lag;
};

RKPositionSteerEngine *RKPositionSteerEngineInit(void);
void RKPositionSteerEngineFree(RKPositionSteerEngine *);

void RKPositionSteerEngineSetVerbose(RKPositionSteerEngine *, const int);
void RKPositionSteerEngineSetInputOutputBuffers(RKPositionSteerEngine *, const RKRadarDesc *,
                                                RKPosition *, uint32_t *,
                                                RKConfig *,   uint32_t *);

int RKPositionSteerEngineStart(RKPositionSteerEngine *);
int RKPositionSteerEngineStop(RKPositionSteerEngine *);

char *RKPositionSteerEngineStatusString(RKPositionSteerEngine *);

#endif
