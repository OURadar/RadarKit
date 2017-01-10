//
//  RKPosition.h
//  RadarKit
//
//  RKPosition provides a wrapper to interact with RadarKit, tag each raw time-series pulse with a set
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
//     you provide the input through RKPositionEngineSetHardwareInitInput(). Typecast it to void *.
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
//     you provide a execution delegate routine through RKPositionEngineSetHardwareExec(). It must be
//     in the form of
//
//         int routine(RKPedestal, const char *);
//
//   - Close the hardware interaction when it is appropriate
//     you provide a resource deallocation delegate routine through RKPositionEngineSetHardwareFree().
//     It must be in the form of
//
//         int routine(RKPedestal);
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKPosition__
#define __RadarKit_RKPosition__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

//#define RKPositionBufferSize    4096

typedef int RKPositionEngineState;
enum RKPositionEngineState {
    RKPositionEngineStateNull,
    RKPositionEngineStateAllocated,
    RKPositionEngineStateActivating,
    RKPositionEngineStateActive,
    RKPositionEngineStateDeactivating,
    RKPositionEngineStateSleep
};

typedef struct rk_position_engine RKPositionEngine;

struct rk_position_engine {
    // User set variables
    char                   name[RKNameLength];
    RKBuffer               pulseBuffer;
    uint32_t               *pulseIndex;
    uint32_t               pulseBufferSize;
    RKPosition             *positionBuffer;
    uint32_t               *positionIndex;
    uint32_t               positionBufferSize;
    uint8_t                verbose;
    RKPedestal             pedestal;
    RKPedestal             (*hardwareInit)(void *);
    int                    (*hardwareExec)(RKPedestal, const char *);
    int                    (*hardwareRead)(RKPedestal, RKPosition *);
    int                    (*hardwareFree)(RKPedestal);
    void                   *hardwareInitInput;

    // Program set variables
    RKClock                *clock;
    double                 *positionTime;
    double                 positionTimeLatest;
    double                 positionTimeOldest;
    pthread_t              threadId;

    // Status / health
    uint32_t               processedPulseIndex;
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    char                   positionStringBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKPositionEngineState  state;
    uint32_t               tic;
    float                  lag;
    uint32_t               almostFull;
    size_t                 memoryUsage;
};

//typedef RKPositionEngine* RKPedestalHandle;

RKPositionEngine *RKPositionEngineInit();
void RKPositionEngineFree(RKPositionEngine *);

void RKPositionEngineSetVerbose(RKPositionEngine *, const int);
void RKPositionEngineSetInputOutputBuffers(RKPositionEngine *,
                                           RKPosition *, uint32_t *, const uint32_t,
                                           RKPulse *,    uint32_t *, const uint32_t);
void RKPositionEngineSetHardwareInit(RKPositionEngine *, RKPedestal(void *), void *);
void RKPositionEngineSetHardwareExec(RKPositionEngine *, int(RKPedestal, const char *));
void RKPositionEngineSetHardwareFree(RKPositionEngine *, int(RKPedestal));

int RKPositionEngineStart(RKPositionEngine *);
int RKPositionEngineStop(RKPositionEngine *);

RKPosition *RKPositionEngineGetVacantPosition(RKPositionEngine *);
void RKPositionEngineSetPositionReady(RKPositionEngine *, RKPosition *);

char *RKPositionEngineStatusString(RKPositionEngine *);
char *RKPositionEnginePositionString(RKPositionEngine *);

#endif /* __RadarKit_RKPedestal__ */
