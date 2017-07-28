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

#ifndef __RadarKit_Position__
#define __RadarKit_Position__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKDSP.h>

typedef struct rk_position_engine RKPositionEngine;

struct rk_position_engine {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    RKBuffer               pulseBuffer;
    uint32_t               *pulseIndex;
    uint32_t               pulseBufferDepth;
    RKPosition             *positionBuffer;
    uint32_t               *positionIndex;
    uint32_t               positionBufferDepth;
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint32_t               configBufferDepth;
    uint8_t                verbose;
    RKPedestal             pedestal;
    RKPedestal             (*hardwareInit)(void *);
    int                    (*hardwareExec)(RKPedestal, const char *);
    int                    (*hardwareRead)(RKPedestal, RKPosition *);
    int                    (*hardwareFree)(RKPedestal);
    void                   *hardwareInitInput;

    // Program set variables
    pthread_t              threadId;
    double                 startTime;

    // Status / health
    uint32_t               processedPulseIndex;
    char                   statusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    char                   positionStringBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    uint32_t               tic;
    float                  lag;
    size_t                 memoryUsage;
};


RKPositionEngine *RKPositionEngineInit();
void RKPositionEngineFree(RKPositionEngine *);

void RKPositionEngineSetVerbose(RKPositionEngine *, const int);
void RKPositionEngineSetInputOutputBuffers(RKPositionEngine *, const RKRadarDesc *,
                                           RKPosition *, uint32_t *,
                                           RKConfig *,   uint32_t *,
                                           RKPulse *,    uint32_t *);

int RKPositionEngineStart(RKPositionEngine *);
int RKPositionEngineStop(RKPositionEngine *);

char *RKPositionEngineStatusString(RKPositionEngine *);
char *RKPositionEnginePositionString(RKPositionEngine *);

#endif /* __RadarKit_RKPedestal__ */
