//
//  RKPedestal.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKPedestal__
#define __RadarKit_RKPedestal__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKPedestalPedzy.h>

#define RKPedestalBufferSize    4096

typedef int RKPedestalEngineState;
enum RKPedestalEngineState {
    RKPedestalEngineStateNull,
    RKPedestalEngineStateAllocated,
    RKPedestalEngineStateActivating,
    RKPedestalEngineStateActive,
    RKPedestalEngineStateDeactivating,
    RKPedestalEngineStateSleep
};

typedef struct rk_pedestal_engine RKPedestalEngine;

struct rk_pedestal_engine {
    // User set variables
    RKPulse                *pulseBuffer;
    uint32_t               *pulseIndex;
    uint32_t               pulseBufferSize;
    RKPedestal             pedestal;
    RKPedestal             (*hardwareInit)(void *);
    int                    (*hardwareExec)(RKPedestal, const char *);
    int                    (*hardwareRead)(RKPedestal, RKPosition *);
    int                    (*hardwareFree)(RKPedestal);
    void                   *hardwareInitInput;

    // Program set variables
    RKPosition             positionBuffer[RKPedestalBufferSize];
    uint32_t               positionIndex;
    uint32_t               positionBufferSize;
    pthread_t              threadId;

    // Status / health
    RKPedestalEngineState  state;
    uint32_t               tic;
    float                  lag;
    uint32_t               almostFull;
    size_t                 memoryUsage;
};

//typedef RKPedestalEngine* RKPedestalHandle;

RKPedestalEngine *RKPedestalEngineInit();
void RKPedestalEngineFree(RKPedestalEngine *);

void RKPedestalEngineSetHardwareToPedzy(RKPedestalEngine *);

void RKPedestalEngineSetHardwareInitInput(void *);
void RKPedestalEngineSetHardwareInit(RKPedestalEngine *, (RKPedestal)(*routine)(void *));
void RKPedestalEngineSetHardwareExec(RKPedestalEngine *, (int)(*routine)(RKPedestal, const char *));
void RKPedestalEngineSetHardwareRead(RKPedestalEngine *, (int)(*routine)(RKPedestal, RKPosition *));
void RKPedestalEngineSetHardwareFree(RKPedestalEngine *, (int)(*routine)(RKPedestal));

int RKPedestalEngineStart(RKPedestalEngine *);
int RKPedestalEngineStop(RKPedestalEngine *);

#endif /* __RadarKit_RKPedestal__ */
