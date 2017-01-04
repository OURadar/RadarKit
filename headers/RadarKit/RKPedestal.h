//
//  RKPedestal.h
//  RadarKit
//
//  RKPedestal provides a wrapper to interact with RadarKit, tag each raw time-series pulse with a set
//  of proper position that contains azimuth and elevation. It manages the run-loop that continuously
//  monitor the incoming position read, where you would supply the actual read function to interpret
//  the binary stream, decode the stream into an RKPosition slot, which is supplied. Each call of the
//  function expects a proper return of RKPosition. If you expect mutiple read for a complete position
//  description, issue your own internal run-loop to satisfy this requirement. This protocol must be
//  strictly followed. It also serves as a bridge to forward the necessary control command, which will
//  be text form. They will be described in the pedestal control language. Finally, a resource clean
//  up routine is called when the program terminates.
//
//  The main protocols are:
//
//   - Initialize a pointer to a structure to communicate with the hardware (void *),
//     you provide the input through RKPedestalEngineSetHardwareInitInput(). Typecast it to void *.
//     It must be in the form of
//
//         RKPedestal routine(void *);
//
//   - A continuous run loop that monitors your input position,
//     you keep filling in the buffer
//
//         int routine(RKPedestal, RKPosition *);
//
//   - Manages and forwards the current control command in the command queue,
//     you provide a execution delegate routine through RKPedestalEngineSetHardwareExec(). It must be
//     in the form of
//
//         int routine(RKPedestal, const char *);
//
//   - Close the hardware interaction when it is appropriate
//     you provide a resource deallocation delegate routine through RKPedestalEngineSetHardwareFree().
//     It must be in the form of
//
//         int routine(RKPedestal);
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKPedestal__
#define __RadarKit_RKPedestal__

#include <RadarKit/RKFoundation.h>

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

void RKPedestalEngineSetInputOutputBuffers(RKPedestalEngine *, RKBuffer, uint32_t *, const uint32_t);
void RKPedestalEngineSetHardwareInit(RKPedestalEngine *, RKPedestal(void *), void *);
void RKPedestalEngineSetHardwareExec(RKPedestalEngine *, int(RKPedestal, const char *));
void RKPedestalEngineSetHardwareFree(RKPedestalEngine *, int(RKPedestal));

int RKPedestalEngineStart(RKPedestalEngine *);
int RKPedestalEngineStop(RKPedestalEngine *);

#endif /* __RadarKit_RKPedestal__ */
