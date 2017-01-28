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
#include <RadarKit/RKClient.h>

typedef struct rk_health_engine RKHealthEngine;

struct rk_health_engine {
    // User defined variables
    char                name[RKNameLength];
    RKHealth            *healthBuffer;
    uint32_t            *healthIndex;
    uint32_t            healthBufferDepth;
};


RKHealthEngine *RKHealthEngineInit();
void RKHealthEngineFree(RKHealthEngine *);

void RKHealthEngineSetVerbose(RKHealthEngine *, const int);
void RKHealthEngineSetInputOutputBuffers(RKHealthEngine *);

void RKHealthEngineSetHardwareInit(RKHealthEngine *, RKHealth(void *), void *);
void RKHealthEngineSetHardwareExec(RKHealthEngine *, int(RKHealth, const char *));
void RKHealthEngineSetHardwareFree(RKHealthEngine *, int(RKHealth));

int RKHealthEngineStart(RKHealthEngine *);
int RKHealthEngineStop(RKHealthEngine *);

int RKHealthEngineStatusString(RKHealthEngine *);

#endif /* __RadarKit_Health__ */
