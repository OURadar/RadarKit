
//
//  RKPositionSteerEngine.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//
//  Most codes are ported from Ming-Duan Tze's VCP implementation
//

#ifndef __RadarKit_PositionSteerEngine__
#define __RadarKit_PositionSteerEngine__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKDSP.h>

typedef int RKVCPProgress;
enum RKVCPProgress {
    RKVCPProgressNone       = 0,        // Not active, ready for next sweep
    RKVCPProgressSetup      = 1,        // Getting ready
    RKVCPProgressReady      = 1 << 1,   // Ready for the next sweep
    RKVCPProgressMiddle     = 1 << 2,   // Middle of a sweep
    RKVCPProgressEnd        = 1 << 3,   // End of a sweep, waiting for pedestal to stop / reposition
    RKVCPProgressMarker     = 1 << 7    // Sweep complete marker
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

typedef int RKVcpHitter;
enum RKVcpHitter {
    RKVcpAtBat              = 0,        // current vcp
    RKVcpPinch              = 1,        // vcp only once
    RKVcpLine               = 2,        // next vcp
};

#define InstructIsAzimuth(i)     ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisAzimuth)
#define InstructIsElevation(i)   ((i & RKPedestalInstructTypeAxisMask) == RKPedestalInstructTypeAxisElevation)
#define InstructIsTest(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeTest)
#define InstructIsSlew(i)        ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeSlew)
#define InstructIsPoint(i)       ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModePoint)
#define InstructIsStandby(i)     ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeStandby)
#define InstructIsDisable(i)     ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeDisable)
#define InstructIsEnable(i)      ((i & RKPedestalInstructTypeModeMask) == RKPedestalInstructTypeModeEnable)

#define RKVCPMaximumSweepCount   256

typedef struct rk_vcp_sweep {
    RKVcpMode                   mode;
    float                       azimuthStart;                   // Azimuth start of a sweep
    float                       azimuthEnd;                     // Azimuth end of a sweep
    float                       azimuthSlew;                    // Azimuth slew speed
    float                       azimuthMark;                    // Azimuth to mark end of sweep
    float                       elevationStart;                 // Elevation start of a sweep
    float                       elevationEnd;                   // Elevation end of a sweep
    float                       elevationSlew;                  // Elevation slew speed
} RKPedestalVcpSweepHandle;

typedef struct rk_vcp {
    // User set parameters
    char                        name[64];
    RKVcpOption                 option;
    RKPedestalVcpSweepHandle    batterSweeps[RKVCPMaximumSweepCount];
    RKPedestalVcpSweepHandle    onDeckSweeps[RKVCPMaximumSweepCount];
    RKPedestalVcpSweepHandle    inTheHoleSweeps[RKVCPMaximumSweepCount];
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
    RKPedestalAction            lastAction;                     // store last action
} RKPedestalVcpHandle;

typedef struct rk_position_steer_engine RKPositionSteerEngine;

struct rk_position_steer_engine {
    // User set variables
    RKName                 name;
    RKRadarDesc            *radarDescription;
    RKPosition             *positionBuffer;
    uint32_t               *positionIndex;
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint8_t                verbose;

    // Program set variables
    pthread_t              threadId;
    double                 startTime;
    bool                   vcpActive;
    int                    vcpIndex;
    int                    vcpSweepCount;
    struct timeval         currentTime;
    struct timeval         statusPeriod;
    struct timeval         statusTriggerTime;

    // Status / health
    size_t                 memoryUsage;
    char                   statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    uint64_t               tic;
    float                  lag;
};

RKPositionSteerEngine *RKPositionSteerEngineInit(void);
void RKPositionSteerEngineFree(RKPositionSteerEngine *);

void RKPositionSteerEngineSetVerbose(RKPositionSteerEngine *, const int);
void RKPositionSteerEngineSetInputOutputBuffers(RKPositionSteerEngine *, const RKRadarDesc *,
                                                RKPosition *, uint32_t *,
                                                RKConfig *,   uint32_t *);

int RKPositionSteerEngineStart(RKPositionSteerEngine *);
int RKPositionSteerEngineStop(RKPositionSteerEngine *);

char *RKPositionSteerEngineStatusString(RKPositionSteerEngine *);

#endif
