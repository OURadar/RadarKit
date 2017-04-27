//
//  RKHealthLogger.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 4/26/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_HealthLogger__
#define __RadarKit_HealthLogger__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>

typedef struct rk_health_logger RKHealthLogger;

struct rk_health_logger {
    // User defined variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    RKHealth               *healthBuffer;
    uint32_t               *healthIndex;
    uint32_t               healthBufferDepth;
    uint8_t                verbose;
    RKHealthRelay          healthRelay;
    RKFileManager          *fileManager;

    // Program set variables
    FILE                   *fid;
    pthread_t              tidBackground;

    // Status / health
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    size_t                 memoryUsage;
};

RKHealthLogger *RKHealthLoggerInit();
void RKHealthLoggerFree(RKHealthLogger *);

void RKHealthLoggerSetVerbose(RKHealthLogger *, const int);
void RKHealthLoggerSetInputOutputBuffers(RKHealthLogger *, RKRadarDesc *, RKFileManager *,
                                         RKHealth *healthBuffer, uint32_t *healthIndex, const uint32_t healthBufferDepth);

int RKHealthLoggerStart(RKHealthLogger *);
int RKHealthLoggerStop(RKHealthLogger *);

char *RKHealthLoggerStatusString(RKHealthLogger *);

#endif