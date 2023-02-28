//
//  RKPedestalPedzy.h
//  RadarKit
//
//  This is an example implementation of hardware interaction through RKPedestal
//  Many features here are specific the pedzy and may not apply to your situations
//
//  Created by Boonleng Cheong on 1/3/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_PedestalPedzy__
#define __RadarKit_PedestalPedzy__

#include <RadarKit/RKRadar.h>

#define RKPedestalPedzyFeedbackDepth   8
#define RKPedestalVcpMaxSweeps         256

#define RKPedestalVcpRepeat            1
#define RKPedestalVcpNoRepeat          0
#define RKPedestalVcpStatusPeriodMS    2000

struct timeval RKPedestalVcpCurrentTime;
struct timeval RKPedestalVcpStatusTriggerTime;
struct timeval RKPedestalVcpStatusPeriod;

enum RKVcpProgress {
    RKVcpProgressNone       = 0,        // Not active, ready for next sweep
    RKVcpProgressSetup      = 1,        // Getting ready
    RKVcpProgressReady      = 1 << 1,   // Ready for the next sweep
    RKVcpProgressMiddle     = 1 << 2,   // Middle of a sweep
    RKVcpProgressEnd        = 1 << 3,   // End of a sweep, waiting for pedestal to stop / reposition
    RKVcpProgressMarker     = 1 << 7    // Sweep complete marker
};

typedef int RKVcpOption;
enum RKVcpOption {
    RKVcpOptionNone                         = 0,
    RKVcpOptionBrakeElevationDuringSweep    = 1,
    RKVcpOptionRepeat                       = 1 << 1,
    RKVcpOptionVerbose                      = 1 << 2
};

typedef int RKVcpMode;
enum RKVcpMode {
    RKVcpModeRHI             = 1,
    RKVcpModeSector          = 2,
    RKVcpModePPI             = 3,
    RKVcpModePPIAzimuthStep  = 4,
    RKVcpModePPIContinuous   = 5,
    RKVcpModeNewSector       = 6,
    RKVcpModeSpeedDown       = 7
};

#define InstructIsAzimuth(i)     ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisAzimuth)
#define InstructIsElevation(i)   ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisElevation)
#define InstructIsTest(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeTest)
#define InstructIsSlew(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeSlew)
#define InstructIsPoint(i)       ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModePoint)
#define InstructIsStandby(i)     ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeStandby)
#define InstructIsDisable(i)     ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeDisable)
#define InstructIsEnable(i)      ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeEnable)

typedef struct rk_vcp_sweep {
    RKVcpMode   mode;
    float       azimuthStart;                   // Azimuth start of a sweep
    float       azimuthEnd;                     // Azimuth end of a sweep
    float       azimuthSlew;                    // Azimuth slew speed
    float       azimuthMark;                    // Azimuth to mark end of sweep
    float       elevationStart;                 // Elevation start of a sweep
    float       elevationEnd;                   // Elevation end of a sweep
    float       elevationSlew;                  // Elevation slew speed
} RKPedestalVcpSweepHandle;

typedef struct rk_vcp {
    // User set parameters
    char                        name[64];
    RKVcpOption                 option;
    RKPedestalVcpSweepHandle    batterSweeps[RKPedestalVcpMaxSweeps];
    RKPedestalVcpSweepHandle    onDeckSweeps[RKPedestalVcpMaxSweeps];
    RKPedestalVcpSweepHandle    inTheHoleSweeps[RKPedestalVcpMaxSweeps];
    int                         inTheHoleCount;
    int                         onDeckCount;
    int                         sweepCount;
    // Program set variables
    bool                        active;
    int                         i;                              // Sweep index for control
    int                         j;                              // Sweep index for marker
    int                         tic;                            // Counter of the run loop
    float                       elevationPrevious;
    float                       azimuthPrevious;
    float                       counterTargetElevation;         // Elevation control target
    float                       counterTargetAzimuth;           // Azimuth control target
    float                       counterMarkerAzimuth;           // Azimuth marker target
    float                       targetElevation;
    float                       targetAzimuth;                  // Target of end sweep: counter or transition aimuth
    float                       targetDiffAzimuthPrevious;
    float                       markerDiffAzimuthPrevious;
    float                       sweepMarkerElevation;           // Elevation marker
    float                       sweepElevation;                 // Elevation control
    float                       sweepAzimuth;                   // Azimuth control
    int                         progress;
} RKPedestalVcpHandle;

typedef struct rk_pedzy {
    // User set variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKPedestalPedzyFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    float                  headingOffset;
    bool                   vcpActive;
    RKPedestalVcpHandle    *vcpHandle;
    uint32_t                lastActionAge;
    // // VCP engine
    // RKVCPEngine            *vcpEngine;

    // Program set variables
    pthread_t              tidPedestalMonitor;
    pthread_t              tidVcpEngine;
} RKPedestalPedzy;

RKPedestal RKPedestalPedzyInit(RKRadar *, void *);
int RKPedestalPedzyExec(RKPedestal, const char *, char *);
int RKPedestalPedzyFree(RKPedestal);
RKPedestalAction pedestalVcpGetAction(RKPedestalVcpHandle *, const RKPosition *);
RKPedestalVcpHandle *pedestalVcpInit(void);
void pedestalVcpArmSweeps(RKPedestalVcpHandle *, const bool);
void pedestalVcpClearSweeps(RKPedestalVcpHandle *);
void pedestalVcpClearHole(RKPedestalVcpHandle *);
void pedestalVcpClearDeck(RKPedestalVcpHandle *);
void pedestalVcpNextHitter(RKPedestalVcpHandle *);
RKPedestalVcpSweepHandle pedestalVcpMakeSweep(RKVcpMode mode,
                       const float el_start, const float el_end,
                       const float az_start, const float az_end, const float az_mark,
                       const float rate);
int pedestalVcpAddLineupSweep(RKPedestalVcpHandle *, RKPedestalVcpSweepHandle );
int pedestalVcpAddPinchSweep(RKPedestalVcpHandle *, RKPedestalVcpSweepHandle );
void makeSweepMessage(RKPedestalVcpSweepHandle *, char *, int SC, bool linetag);
void pedestalVcpSummary(RKPedestalVcpHandle *, char *);
#endif /* __RadarKit_RKPedestal__ */
