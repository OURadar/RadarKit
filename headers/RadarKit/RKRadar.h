//
//  RKRadar.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_Radar__
#define __RadarKit_Radar__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKServer.h>
#include <RadarKit/RKClient.h>
#include <RadarKit/RKClock.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKHealth.h>
#include <RadarKit/RKPosition.h>
#include <RadarKit/RKPulseCompression.h>
#include <RadarKit/RKMoment.h>
#include <RadarKit/RKHealthLogger.h>
#include <RadarKit/RKDataRecorder.h>
#include <RadarKit/RKSweep.h>
#include <RadarKit/RKWaveform.h>
#include <RadarKit/RKPreference.h>
#include <RadarKit/RKFileManager.h>
#include <RadarKit/RKRadarRelay.h>

typedef uint32_t RKRadarState;                                     // Everything allocated and live: 0x81ff0555
enum RKRadarState {
    RKRadarStateBaseAllocated                        = 1,          // Base
    RKRadarStateRayBufferAllocating                  = (1 << 1),   // Data buffers
    RKRadarStateRayBufferInitialized                 = (1 << 2),   //
    RKRadarStateRawIQBufferAllocating                = (1 << 3),   //
    RKRadarStateRawIQBufferInitialized               = (1 << 4),   //
    RKRadarStateConfigBufferAllocating               = (1 << 5),   //
    RKRadarStateConfigBufferInitialized              = (1 << 6),   //
    RKRadarStateHealthBufferAllocating               = (1 << 7),
    RKRadarStateHealthBufferInitialized              = (1 << 8),
    RKRadarStateHealthNodesAllocating                = (1 << 9),
    RKRadarStateHealthNodesInitialized               = (1 << 10),
    RKRadarStatePositionBufferAllocating             = (1 << 11),
    RKRadarStatePositionBufferInitialized            = (1 << 12),
    RKRadarStateControlsAllocating                   = (1 << 13),
    RKRadarStateControlsInitialized                  = (1 << 14),
    RKRadarStatePulseCompressionEngineInitialized    = (1 << 16),  // Engines
    RKRadarStatePositionEngineInitialized            = (1 << 17),
    RKRadarStateHealthEngineInitialized              = (1 << 18),
    RKRadarStateMomentEngineInitialized              = (1 << 19),
    RKRadarStateSweepEngineInitialized               = (1 << 20),
    RKRadarStateFileRecorderInitialized              = (1 << 21),
    RKRadarStateHealthLoggerInitialized              = (1 << 22),  // Recorders
    RKRadarStateFileManagerInitialized               = (1 << 23),
    RKRadarStateRadarRelayInitialized                = (1 << 24),
    RKRadarStateHealthRelayInitialized               = (1 << 25),
    RKRadarStateTransceiverInitialized               = (1 << 26),
    RKRadarStatePedestalInitialized                  = (1 << 27),
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
    char                       name[RKNameLength];                   // Name of the engine
    RKRadarDesc                desc;
    RKRadarState               state;
    bool                       active;
    size_t                     memoryUsage;
    //
    // Buffers
    //
    RKStatus                   *status;                              // Overall RadarKit engine status
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
    RKHealthNode               healthNodeCount;
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
    RKHealthLogger             *healthLogger;
    RKSweepEngine              *sweepEngine;
    RKDataRecorder             *dataRecorder;
    RKFileManager              *fileManager;
    RKRadarRelay               *radarRelay;
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
    //
    RKMasterController         masterController;
    int                        (*masterControllerExec)(RKMasterController, const char *, char *);
    //
    // Controls
    //
    RKControl                  *controls;
    uint32_t                   controlIndex;
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
RKRadar *RKInitAsRelay(void);
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
int RKSetDataPath(RKRadar *, const char *path);
int RKSetDataUsageLimit(RKRadar *, const size_t limit);
int RKSetDoNotWrite(RKRadar *, const bool doNotWrite);

// Some operating parameters
int RKSetWaveform(RKRadar *, RKWaveform *);
int RKSetWaveformByFilename(RKRadar *, const char *);
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
int RKResetEngines(RKRadar *);
void RKPerformMasterTaskInBackground(RKRadar *, const char *);

void RKMeasureNoise(RKRadar *);
void RKSetSNRThreshold(RKRadar *radar, const RKFloat);

// Healths
RKHealthNode RKRequestHealthNode(RKRadar *);
RKHealth *RKGetVacantHealth(RKRadar *, const RKHealthNode node);
void RKSetHealthReady(RKRadar *, RKHealth *);
RKHealth *RKGetLatestHealth(RKRadar *);
int RKGetEnumFromLatestHealth(RKRadar *, const char *keyword);

// Positions
RKPosition *RKGetVacantPosition(RKRadar *);
void RKSetPositionReady(RKRadar *, RKPosition *);
RKPosition *RKGetLatestPosition(RKRadar *);

// Pulses
RKPulse *RKGetVacantPulse(RKRadar *);
void RKSetPulseHasData(RKRadar *, RKPulse *);
void RKSetPulseReady(RKRadar *, RKPulse *);
RKPulse *RKGetLatestPulse(RKRadar *radar);

// Rays
RKRay *RKGetVacantRay(RKRadar *);
void RKSetRayReady(RKRadar *, RKRay *);

// Configs
void RKAddConfig(RKRadar *radar, ...);
RKConfig *RKGetLatestConfig(RKRadar *radar);

// Controls
void RKAddControl(RKRadar *, const char *label, const char *command);
void RKUpdateControl(RKRadar *, uint8_t, const char *label, const char *command);

// Absolute address value query
void RKGetRegisterValue(RKRadar *radar, void *value, const unsigned long registerOffset, size_t size);
void RKSetRegisterValue(RKRadar *radar, void *value, const unsigned long registerOffset, size_t size);
void RKShowOffsets(RKRadar *radar);

#endif /* defined(__RadarKit_RKRadar__) */
