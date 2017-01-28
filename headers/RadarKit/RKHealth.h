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

typedef int RKHealthEngineState;
enum RKHealthEngineState {
    RKHealthEngineStateNull,
    RKHealthEngineStateAllocated,
    RKHealthEngineStateActivating,
    RKHealthEngineStateActive,
    RKHealthEngineStateDeactivating,
    RKHealthEngineStateSleep
};

typedef struct rk_health_engine RKHealthEngine;

struct rk_health_engine {
    // User defined variables
    char                   name[RKNameLength];
    RKHealth               *healthBuffer;
    uint32_t               *healthIndex;
    uint32_t               healthBufferDepth;
    uint8_t                verbose;
    RKHealthRelay          healthRelay;
    RKHealthRelay          (*hardwareInit)(void *);
    int                    (*hardwareExec)(RKHealthRelay, const char *);
    int                    (*hardwareRead)(RKHealthRelay, RKHealth *);
    int                    (*hardwareFree)(RKHealthRelay);
    void                   *hardwareInitInput;

    // Program set variables
    pthread_t              threadId;
    double                 startTime;

    // Status / health
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKHealthEngineState    state;
    size_t                 memoryUsage;
};


RKHealthEngine *RKHealthEngineInit();
void RKHealthEngineFree(RKHealthEngine *);

void RKHealthEngineSetVerbose(RKHealthEngine *, const int);
void RKHealthEngineSetInputOutputBuffers(RKHealthEngine *,
                                         RKHealth *healthBuffer, uint32_t *healthIndex, const uint32_t healthBufferDepth);

//void RKHealthEngineSetHardwareInit(RKHealthEngine *, RKHealth(void *), void *);
//void RKHealthEngineSetHardwareExec(RKHealthEngine *, int(RKHealth, const char *));
//void RKHealthEngineSetHardwareFree(RKHealthEngine *, int(RKHealth));

int RKHealthEngineStart(RKHealthEngine *);
int RKHealthEngineStop(RKHealthEngine *);

char *RKHealthEngineStatusString(RKHealthEngine *);

#endif /* __RadarKit_Health__ */
