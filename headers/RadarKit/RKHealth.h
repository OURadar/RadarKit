//
//  RKHealth.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Health__
#define __RadarKit_Health__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>
#include <RadarKit/RKClient.h>

typedef struct rk_health_engine RKHealthEngine;

struct rk_health_engine {
    // User defined variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKNodalHealth                    *healthNodes;
    RKHealth                         *healthBuffer;
    uint32_t                         *healthIndex;
    uint8_t                          verbose;

    // Program set variables
    FILE                             *fid;
    pthread_t                        tidHealthConsolidator;

    // Status / health
    char                             statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t                         statusBufferIndex;
    RKEngineState                    state;
	uint64_t                         tic;
    size_t                           memoryUsage;
};

RKHealthEngine *RKHealthEngineInit(void);
void RKHealthEngineFree(RKHealthEngine *);

void RKHealthEngineSetVerbose(RKHealthEngine *, const int);
void RKHealthEngineSetInputOutputBuffers(RKHealthEngine *, const RKRadarDesc *,
                                         RKNodalHealth *healthNodes,
                                         RKHealth *healthBuffer, uint32_t *healthIndex);

int RKHealthEngineStart(RKHealthEngine *);
int RKHealthEngineStop(RKHealthEngine *);

char *RKHealthEngineStatusString(RKHealthEngine *);

#endif /* __RadarKit_Health__ */
