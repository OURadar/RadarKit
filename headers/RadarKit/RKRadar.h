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
#include <RadarKit/RKPulseRingFilter.h>
#include <RadarKit/RKMoment.h>
#include <RadarKit/RKHealthLogger.h>
#include <RadarKit/RKRawDataRecorder.h>
#include <RadarKit/RKSweep.h>
#include <RadarKit/RKProduct.h>
#include <RadarKit/RKWaveform.h>
#include <RadarKit/RKPreference.h>
#include <RadarKit/RKFileManager.h>
#include <RadarKit/RKRadarRelay.h>
#include <RadarKit/RKHostMonitor.h>

#define xstr(s) str(s)
#define str(s) #s
#define RADAR_VARIABLE_OFFSET(STRING, NAME) \
sprintf(STRING, "                    radar->" xstr(NAME) " @ %ld -> %p\n", (unsigned long)((void *)&radar->NAME - (void *)radar), (unsigned int *)&radar->NAME)

typedef uint32_t RKRadarState;                                     // Everything allocated and live: 0x81ff0555
enum RKRadarState {
    RKRadarStateRayBufferAllocated                   = (1 << 0),   // Data buffers
    RKRadarStateRawIQBufferAllocated                 = (1 << 1),   //
    RKRadarStateStatusBufferAllocated                = (1 << 2),   //
    RKRadarStateConfigBufferAllocated                = (1 << 3),   //
    RKRadarStateHealthBufferAllocated                = (1 << 4),   //
    RKRadarStateHealthNodesAllocated                 = (1 << 5),   //
    RKRadarStatePositionBufferAllocated              = (1 << 6),   //
    RKRadarStateWaveformCalibrationsAllocated        = (1 << 7),   //
    RKRadarStateControlsAllocated                    = (1 << 8),   //
    RKRadarStateProductBufferAllocated               = (1 << 9),   //
    RKRadarStatePulseCompressionEngineInitialized    = (1 << 16),  // Engines
    RKRadarStatePulseRingFilterEngineInitialized     = (1 << 17),  //
    RKRadarStatePositionEngineInitialized            = (1 << 18),  //
    RKRadarStateHealthEngineInitialized              = (1 << 19),  //
    RKRadarStateMomentEngineInitialized              = (1 << 20),  //
    RKRadarStateSweepEngineInitialized               = (1 << 21),  //
    RKRadarStateFileRecorderInitialized              = (1 << 22),  //
    RKRadarStateHealthLoggerInitialized              = (1 << 23),  // Recorders
    RKRadarStateFileManagerInitialized               = (1 << 24),
    RKRadarStateHealthRelayInitialized               = (1 << 25),
    RKRadarStateTransceiverInitialized               = (1 << 26),
    RKRadarStatePedestalInitialized                  = (1 << 27),
    RKRadarStateHostMonitorInitialized               = (1 << 28),
    RKRadarStateRadarRelayInitialized                = (1 << 30),
    RKRadarStateLive                                 = (1 << 31)
};

typedef struct rk_radar RKRadar;
struct rk_radar {
    //
    // General attributes
    //
    RKName                           name;                           // Name of the engine
    RKRadarDesc                      desc;
    RKRadarState                     state;
    bool                             active;
    size_t                           memoryUsage;
    uint8_t                          processorCount;
    uint64_t                         tic;
    pthread_mutex_t                  mutex;
    //
    // Buffers
    //
    RKStatus                         *status;                        // Overall RadarKit engine status
    RKConfig                         *configs;
    RKHealth                         *healths;
    RKPosition                       *positions;
    RKBuffer                         pulses;
    RKBuffer                         rays;
    RKProduct                        *products;
    //
    // Anchor indices of the buffers
    //
    uint32_t                         statusIndex;
    uint32_t                         configIndex;
    uint32_t                         healthIndex;
    uint32_t                         positionIndex;
    uint32_t                         pulseIndex;
    uint32_t                         rayIndex;
    uint32_t                         productIndex;
    //
    // Secondary Health Buffer
    //
    RKHealthNode                     healthNodeCount;
    RKNodalHealth                    *healthNodes;
    //
    //
    // Internal engines
    //
    RKClock                          *pulseClock;
    RKClock                          *positionClock;
    RKHealthEngine                   *healthEngine;
    RKPositionEngine                 *positionEngine;
    RKPulseCompressionEngine         *pulseCompressionEngine;
    RKPulseRingFilterEngine          *pulseRingFilterEngine;
    RKMomentEngine                   *momentEngine;
    RKRawDataRecorder                *rawDataRecorder;
    RKHealthLogger                   *healthLogger;
    RKSweepEngine                    *sweepEngine;
    RKFileManager                    *fileManager;
    RKRadarRelay                     *radarRelay;
    RKHostMonitor                    *hostMonitor;
    //
    // System Inspector
    //
    RKSimpleEngine                   *systemInspector;
    //
    // Internal copies of things
    //
    RKWaveform                       *waveform;
    RKIIRFilter                      *filter;
    //
    // Hardware protocols for controls
    //
    RKTransceiver                    transceiver;
    RKTransceiver                    (*transceiverInit)(RKRadar *, void *);
    int                              (*transceiverExec)(RKTransceiver, const char *, char *);
    int                              (*transceiverFree)(RKTransceiver);
    void                             *transceiverInitInput;
    char                             transceiverResponse[RKMaximumStringLength];
    //
    RKPedestal                       pedestal;
    RKPedestal                       (*pedestalInit)(RKRadar *, void *);
    int                              (*pedestalExec)(RKPedestal, const char *, char *);
    int                              (*pedestalFree)(RKPedestal);
    void                             *pedestalInitInput;
    char                             pedestalResponse[RKMaximumStringLength];
    //
    RKHealthRelay                    healthRelay;
    RKHealthRelay                    (*healthRelayInit)(RKRadar *, void *);
    int                              (*healthRelayExec)(RKHealthRelay, const char *, char *);
    int                              (*healthRelayFree)(RKHealthRelay);
    void                             *healthRelayInitInput;
    char                             healthRelayResponse[RKMaximumStringLength];
    //
    RKMasterController               masterController;
    int                              (*masterControllerExec)(RKMasterController, const char *, char *);
    //
    // Waveform calibrations
    //
    RKWaveformCalibration            *waveformCalibrations;
    uint32_t                         waveformCalibrationCount;
    //
    // Controls
    //
    RKControl                        *controls;
    uint32_t                         controlCount;
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
// Hardware Hooks
//
#pragma mark - Hardware Hooks

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

//
// Properties
//
#pragma mark - Before-Live Properties

// These can only be set before the radar goes live
int RKSetProcessingCoreCounts(RKRadar *, const unsigned int, const unsigned int);

#pragma mark - Properties

// Some states of the radar
int RKSetVerbosity(RKRadar *, const int);
int RKSetVerbosityUsingArray(RKRadar *, const uint8_t *);
int RKSetDataPath(RKRadar *, const char *);
int RKSetDataUsageLimit(RKRadar *, const size_t limit);
int RKSetRecordingLevel(RKRadar *, const int);
int RKToggleRawDataRecorderMode(RKRadar *);

// Some operating parameters
int RKSetWaveform(RKRadar *, RKWaveform *);
int RKSetWaveformByFilename(RKRadar *, const char *);
int RKSetWaveformToImpulse(RKRadar *);
int RKSetPRF(RKRadar *, const uint32_t);
uint32_t RKGetPulseCapacity(RKRadar *);

// If there is a tic count from firmware, use it as clean reference for time derivation
void RKSetPulseTicsPerSeconds(RKRadar *, const double);
void RKSetPositionTicsPerSeconds(RKRadar *, const double);

// Moment processor
int RKSetMomentProcessorToMultiLag(RKRadar *, const uint8_t);
int RKSetMomentProcessorToPulsePair(RKRadar *);
int RKSetMomentProcessorToPulsePairHop(RKRadar *);
int RKSetMomentProcessorRKPulsePairStaggeredPRT(RKRadar *);

// Moment recorder (RadarKit uses netcdf by default)
int RKSetProductRecorder(RKRadar *radar, int (*productRecorder)(RKProduct *, char *));

// Pulse ring filter (FIR / IIR ground clutter filter)
int RKSetPulseRingFilterByType(RKRadar *, RKFilterType, const uint32_t);
int RKSetPulseRingFilter(RKRadar *, RKIIRFilter *, const uint32_t);

//
// Interactions
//
#pragma mark - Interaction / State Change

// State
int RKGoLive(RKRadar *);
int RKWaitWhileActive(RKRadar *);
int RKStart(RKRadar *);
int RKStop(RKRadar *);
int RKSoftRestart(RKRadar *);
int RKResetClocks(RKRadar *);
int RKExecuteCommand(RKRadar *, const char *, char *);
void RKPerformMasterTaskInBackground(RKRadar *, const char *);

// General
void RKMeasureNoise(RKRadar *);
void RKSetSNRThreshold(RKRadar *, const RKFloat);

// Status
RKStatus *RKGetVacantStatus(RKRadar *);
void RKSetStatusReady(RKRadar *, RKStatus *);

// Configs
void RKAddConfig(RKRadar *radar, ...);
RKConfig *RKGetLatestConfig(RKRadar *radar);

// Healths
RKHealthNode RKRequestHealthNode(RKRadar *);
RKHealth *RKGetVacantHealth(RKRadar *, const RKHealthNode);
void RKSetHealthReady(RKRadar *, RKHealth *);
RKHealth *RKGetLatestHealth(RKRadar *);
RKHealth *RKGetLatestHealthOfNode(RKRadar *, const RKHealthNode);
int RKGetEnumFromLatestHealth(RKRadar *, const char *);

// Positions
RKPosition *RKGetVacantPosition(RKRadar *);
void RKSetPositionReady(RKRadar *, RKPosition *);
RKPosition *RKGetLatestPosition(RKRadar *);
float RKGetPositionUpdateRate(RKRadar *);

// Pulses
RKPulse *RKGetVacantPulse(RKRadar *);
void RKSetPulseHasData(RKRadar *, RKPulse *);
void RKSetPulseReady(RKRadar *, RKPulse *);
RKPulse *RKGetLatestPulse(RKRadar *);

// Rays
RKRay *RKGetVacantRay(RKRadar *);
void RKSetRayReady(RKRadar *, RKRay *);
RKRay *RKGetLatestRay(RKRadar *);

// Waveform Calibrations
void RKAddWaveformCalibration(RKRadar *, const RKWaveformCalibration *);
void RKUpdateWaveformCalibration(RKRadar *, const uint8_t, const RKWaveformCalibration *);
void RKClearWaveformCalibrations(RKRadar *);
void RKConcludeWaveformCalibrations(RKRadar *);

// Controls
void RKAddControl(RKRadar *, const RKControl *);
void RKAddControlAsLabelAndCommand(RKRadar *radar, const char *label, const char *command);
void RKUpdateControl(RKRadar *, const uint8_t, const RKControl *);
void RKClearControls(RKRadar *);
void RKConcludeControls(RKRadar *);

#pragma mark - Developer Access

// Absolute address value query
void RKGetRegisterValue(RKRadar *, void *value, const unsigned long registerOffset, size_t size);
void RKSetRegisterValue(RKRadar *, void *value, const unsigned long registerOffset, size_t size);
void RKShowOffsets(RKRadar *, char *);
int RKBufferOverview(RKRadar *, char *, const RKOverviewFlag);

#endif /* defined(__RadarKit_RKRadar__) */
