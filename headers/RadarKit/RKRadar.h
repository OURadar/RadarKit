//
//  RKRadar.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKRadar__
#define __RadarKit_RKRadar__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKServer.h>
#include <RadarKit/RKClient.h>
#include <RadarKit/RKClock.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKHealth.h>
#include <RadarKit/RKPosition.h>
#include <RadarKit/RKPulseCompression.h>
#include <RadarKit/RKMoment.h>
#include <RadarKit/RKSweep.h>

typedef uint32_t RKRadarState;
enum RKRadarState {
    RKRadarStateBaseAllocated                        = 1,          // 0x01
    RKRadarStateRayBufferAllocating                  = (1 << 1),   // 0x02
    RKRadarStateRayBufferInitialized                 = (1 << 2),   // 0x04
    RKRadarStateRawIQBufferAllocating                = (1 << 3),   // 0x08
    RKRadarStateRawIQBufferInitialized               = (1 << 4),   // 0x10
    RKRadarStateConfigBufferAllocating               = (1 << 5),   // 0x20
    RKRadarStateConfigBufferInitialized              = (1 << 6),   // 0x40
    RKRadarStateHealthBufferAllocating               = (1 << 7),
    RKRadarStateHealthBufferInitialized              = (1 << 8),
    RKRadarStatePositionBufferAllocating             = (1 << 9),
    RKRadarStatePositionBufferInitialized            = (1 << 10),
    RKRadarStatePulseCompressionEngineInitialized    = (1 << 16),
    RKRadarStatePositionEngineInitialized            = (1 << 17),
    RKRadarStateHealthEngineInitialized              = (1 << 18),
    RKRadarStateMomentEngineInitialized              = (1 << 19),
    RKRadarStateSweepEngineInitialized               = (1 << 20),
    RKRadarStateHealthRelayInitialized               = (1 << 21),
    RKRadarStateTransceiverInitialized               = (1 << 22),
    RKRadarStatePedestalInitialized                  = (1 << 23),
    RKRadarStateLive                                 = (1 << 31)
};

/*!
 * @typedef RKRadar
 * @brief The big structure that holds all the necessary buffers
 * @param rawPulse
 */
typedef struct rk_radar RKRadar;
struct rk_radar {
    //
    // General buffers
    //
    char                       name[RKNameLength];
    //
    // Buffers
    //
    RKConfig                   *configs;
    RKHealth                   *healths;
    RKPosition                 *positions;
    RKBuffer                   pulses;
    RKBuffer                   rays;
    //
    // Anchor indices of the buffers
    //
    uint32_t                   configIndex;
    uint32_t                   healthIndex;
    uint32_t                   positionIndex;
    uint32_t                   pulseIndex;
    uint32_t                   rayIndex;
    //
    //
    RKRadarDesc                desc;
    RKRadarState               state;
    bool                       active;
    //
    size_t                     memoryUsage;
    //
    // Internal engines
    //
    RKClock                    *pulseClock;
    RKClock                    *positionClock;
    RKHealthEngine             *healthEngine;
    RKPositionEngine           *positionEngine;
    RKPulseCompressionEngine   *pulseCompressionEngine;
    RKMomentEngine             *momentEngine;
    RKSweepEngine              *sweepEngine;
    //
    pthread_t                  monitorThreadId;
    //
    // Hardware protocols for controls
    //
    RKTransceiver              transceiver;
    RKTransceiver              (*transceiverInit)(RKRadar *, void *);
    int                        (*transceiverExec)(RKTransceiver, const char *);
    int                        (*transceiverFree)(RKTransceiver);
    void                       *transceiverInitInput;
    pthread_t                  transceiverThreadId;
    //
    RKPedestal                 pedestal;
    RKPedestal                 (*pedestalInit)(RKRadar *, void *);
    int                        (*pedestalExec)(RKPedestal, const char *);
    int                        (*pedestalFree)(RKPedestal);
    void                       *pedestalInitInput;
    pthread_t                  pedestalThreadId;
    //
    RKHealthRelay              healthRelay;
    RKHealthRelay              (*healthRelayInit)(RKRadar *, void *);
    int                        (*healthRelayExec)(RKHealthRelay, const char *);
    int                        (*healthRelayFree)(RKHealthRelay);
    void                       *healthRelayInitInput;
    pthread_t                  healthRelayThreadId;
};

//
// Life Cycle
//

RKRadar *RKInitWithDesc(RKRadarDesc);
RKRadar *RKInitQuiet(void);
RKRadar *RKInitLean(void);
RKRadar *RKInitMean(void);
RKRadar *RKInitFull(void);
RKRadar *RKInit(void);
int RKFree(RKRadar *radar);

//
// Properties
//

// Set the transceiver. Pass in function pointers: init, exec and free
int RKSetTransceiver(RKRadar *,
                     void *initInput,
                     RKTransceiver initRoutine(RKRadar *, void *),
                     int execRoutine(RKTransceiver, const char *),
                     int freeRoutine(RKTransceiver));

// Set the pedestal. Pass in function pointers: init, exec and free
int RKSetPedestal(RKRadar *,
                  void *initInput,
                  RKPedestal initRoutine(RKRadar *, void *),
                  int execRoutine(RKPedestal, const char *),
                  int freeRoutine(RKPedestal));

// Set the health relay. Pass in function pointers: init, exec and free
int RKSetHealthRelay(RKRadar *,
                     void *initInput,
                     RKHealthRelay initRoutine(RKRadar *, void *),
                     int execRoutine(RKHealthRelay, const char *),
                     int freeRoutine(RKHealthRelay));

// Some states of the radar
int RKSetVerbose(RKRadar *radar, const int verbose);

// Some operating parameters
int RKSetWaveform(RKRadar *, const char *filename, const int group, const int maxDataLength);
int RKSetWaveformToImpulse(RKRadar *);
int RKSetWaveformTo121(RKRadar *);
int RKSetProcessingCoreCounts(RKRadar *,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount);
int RKSetPRF(RKRadar *, const uint32_t prf);
uint32_t RKGetPulseCapacity(RKRadar *);


// If there is a tic count from firmware, use it as clean reference for time derivation
void RKSetPulseTicsPerSeconds(RKRadar *radar, const double delta);
void RKSetPositionTicsPerSeconds(RKRadar *radar, const double delta);

//
// Interactions
//

int RKGoLive(RKRadar *);
int RKWaitWhileActive(RKRadar *);
int RKStop(RKRadar *);

// Healths
RKHealth *RKGetVacantHealth(RKRadar *);
void RKSetHealthReady(RKRadar *, RKHealth *);

// Positions
RKPosition *RKGetVacantPosition(RKRadar *);
void RKSetPositionReady(RKRadar *, RKPosition *);

// Pulses
RKPulse *RKGetVacantPulse(RKRadar *);
void RKSetPulseHasData(RKRadar *, RKPulse *);
void RKSetPulseReady(RKRadar *, RKPulse *);

// Rays
RKRay *RKGetVacantRay(RKRadar *);
void RKSetRayReady(RKRadar *, RKRay *);

void RKAddConfig(RKRadar *radar, ...);

#endif /* defined(__RadarKit_RKRadar__) */
