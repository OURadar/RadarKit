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
#include <RadarKit/RKFile.h>
#include <RadarKit/RKWaveform.h>

typedef uint32_t RKRadarState;                                     // Everything allocated and live: 0x81ff0555
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
    RKRadarStateFileEngineInitialized                = (1 << 21),
    RKRadarStateHealthRelayInitialized               = (1 << 22),
    RKRadarStateTransceiverInitialized               = (1 << 23),
    RKRadarStatePedestalInitialized                  = (1 << 24),
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
    // General attributes
    //
    char                       name[RKNameLength];
    RKRadarDesc                desc;
    RKRadarState               state;
    bool                       active;
    size_t                     memoryUsage;
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
    // Secondary Health Buffer
    //
    RKNodalHealth              *healthNodes;
    //
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
    RKFileEngine               *fileEngine;
    //
    // Internal copies of things
    //
    RKWaveform                 *waveform;
    //
    pthread_mutex_t            mutex;
    //
    // Hardware protocols for controls
    //
    RKTransceiver              transceiver;
    RKTransceiver              (*transceiverInit)(RKRadar *, void *);
    int                        (*transceiverExec)(RKTransceiver, const char *, char *);
    int                        (*transceiverFree)(RKTransceiver);
    void                       *transceiverInitInput;
    char                       transceiverResponse[RKMaximumStringLength];
    //
    RKPedestal                 pedestal;
    RKPedestal                 (*pedestalInit)(RKRadar *, void *);
    int                        (*pedestalExec)(RKPedestal, const char *, char *);
    int                        (*pedestalFree)(RKPedestal);
    void                       *pedestalInitInput;
    char                       pedestalResponse[RKMaximumStringLength];
    //
    RKHealthRelay              healthRelay;
    RKHealthRelay              (*healthRelayInit)(RKRadar *, void *);
    int                        (*healthRelayExec)(RKHealthRelay, const char *, char *);
    int                        (*healthRelayFree)(RKHealthRelay);
    void                       *healthRelayInitInput;
    char                       healthRelayResponse[RKMaximumStringLength];
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
                     int execRoutine(RKTransceiver, const char *, char *),
                     int freeRoutine(RKTransceiver));

// Set the pedestal. Pass in function pointers: init, exec and free
int RKSetPedestal(RKRadar *,
                  void *initInput,
                  RKPedestal initRoutine(RKRadar *, void *),
                  int execRoutine(RKPedestal, const char *, char *),
                  int freeRoutine(RKPedestal));

// Set the health relay. Pass in function pointers: init, exec and free
int RKSetHealthRelay(RKRadar *,
                     void *initInput,
                     RKHealthRelay initRoutine(RKRadar *, void *),
                     int execRoutine(RKHealthRelay, const char *, char *),
                     int freeRoutine(RKHealthRelay));

// Some states of the radar
int RKSetVerbose(RKRadar *, const int verbose);
int RKSetDoNotWrite(RKRadar *, const bool doNotWrite);

// Some operating parameters
int RKSetWaveform(RKRadar *, RKWaveform *waveform, const int origin, const int maxDataLength);
int RKSetWaveformByFilename(RKRadar *, const char *filename, const int group, const int maxDataLength);
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
uint8_t RKRequestHealthNode(RKRadar *);
RKHealth *RKGetVacantHealth(RKRadar *, const RKHealthNode);
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
