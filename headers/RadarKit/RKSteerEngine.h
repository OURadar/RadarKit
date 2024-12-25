
//
//  RKSteerEngine.h
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//
//  Most codes are ported from Ming-Duan Tze's VCP implementation
//

#ifndef __RadarKit_Position_Steer_Engine__
#define __RadarKit_Position_Steer_Engine__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKConfig.h>
#include <RadarKit/RKDSP.h>

#define RKPedestalPositionTolerance    0.1f
#define RKPedestalVelocityTolerance    0.25f
#define RKPedestalPointTimeOut         1500
#define RKPedestalActionPeriod         0.05

#define RKScanModeString(x) x == RKScanModeRHI ? "RHI" : ( \
x == RKScanModePPI ? "PPI" : ( \
x == RKScanModeSector ? "SEC" : ( \
x == RKScanModePoint ? "SPT" : "UNK")))

typedef bool RKScanRepeat;
enum {
    RKScanRepeatNone                             = false,
    RKScanRepeatForever                          = true
};

typedef uint8_t RKPedestalAxis;
enum {
    RKPedestalAxisElevation                      = 0,
    RKPedestalAxisAzimuth                        = 1
};

typedef uint8_t RKScanProgress;
enum {
    RKScanProgressNone                           = 0,                          // Not active, ready for next sweep
    RKScanProgressSetup                          = 1,                          // Getting ready
    RKScanProgressReady                          = 1 << 1,                     // Ready for the next sweep (deprecating)
    RKScanProgressMiddle                         = 1 << 2,                     // Middle of a sweep
    RKScanProgressEnd                            = 1 << 3,                     // End of a sweep, waiting for pedestal to stop / reposition
    RKScanProgressStopPedestal                   = 1 << 4                      // Stop pedestal
};

typedef uint8_t RKScanOption;
enum {
    RKScanOptionNone                             = 0,
    RKScanOptionRepeat                           = 1,
    RKScanOptionVerbose                          = 1 << 1,
    RKScanOptionUsePoint                         = 1 << 2
};

typedef uint8_t RKScanMode;
enum {
    RKScanModeNone,
    RKScanModeRHI,
    RKScanModePPI,
    RKScanModeSector,
    RKScanModePoint,
    RKScanModeSpeedDown        // need to learn more
};

typedef uint8_t RKScanHitter;
enum {
    RKScanAtBat                                  = 0,                          // current vcp
    RKScanPinch                                  = 1,                          // vcp only once
    RKScanLine                                   = 2,                          // next vcp
};

typedef uint16_t RKSteerCommand;
enum {
    RKSteerCommandNone,
    RKSteerCommandHome,
    RKSteerCommandPoint,
    RKSteerCommandPPISet,
    RKSteerCommandRHISet,
    RKSteerCommandVolume,
    RKSteerCommandMotionCount,
    RKSteerCommandSummary,
    RKSteerCommandScanStart,
    RKSteerCommandScanStop,
    RKSteerCommandScanNext,
    RKSteerCommandOnce                           = (1 << 8),
    RKSteerCommandImmediate                      = (1 << 9),
    RKSteerCommandRelative                       = (1 << 10),
    RKSteerCommandProperty4                      = (1 << 11),
    RKSteerCommandPropertyMask                   = 0xFF00,                     // Bits > 8 for properties like once / immediate
    RKSteerCommandInstructionMask                = 0x00FF
};

#define RKSteerCommandIsMotion(x)    (((x) & RKSteerCommandInstructionMask) < RKSteerCommandSummary)
#define RKSteerCommandIsOnce(x)      ((x) & RKSteerCommandOnce)
#define RKSteerCommandIsImmediate(x) ((x) & RKSteerCommandImmediate)
#define RKSteerCommandIsRelative(x)  ((x) & RKSteerCommandRelative)

typedef struct rk_scan_path {
    RKScanMode                  mode;                                          // Scan mode
    float                       azimuthStart;                                  // Azimuth start of a scan
    float                       azimuthEnd;                                    // Azimuth end of a scan
    float                       azimuthSlew;                                   // Azimuth slew speed
    float                       azimuthMark;                                   // Azimuth to mark end of scan
    float                       elevationStart;                                // Elevation start of a scan
    float                       elevationEnd;                                  // Elevation end of a scan
    float                       elevationSlew;                                 // Elevation slew speed
} RKScanPath;

typedef struct rk_scan_object {
    RKName                      name;
    RKScanOption                option;
    RKScanPath                  batterScans[RKMaximumScanCount];
    RKScanPath                  onDeckScans[RKMaximumScanCount];
    RKScanPath                  inTheHoleScans[RKMaximumScanCount];
    uint16_t                    inTheHoleCount;
    uint16_t                    onDeckCount;
    uint16_t                    sweepCount;
    bool                        active;
    int                         i;                                             // Sweep index for control
    int                         tic;                                           // Counter of the run loop
    int                         toc;                                           //
    float                       elevationPrevious;                             //
    float                       azimuthPrevious;                               //
    RKScanProgress              progress;                                      //
    RKScanAction                lastAction;                                    // store last action
} RKScanObject;

typedef struct rk_position_steer_engine RKSteerEngine;

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
    RKScanObject           vcpHandle;
    RKScanAction           actions[RKPedestalActionBufferDepth];
    uint32_t               actionIndex;
    uint32_t               volumeIndex;
    uint32_t               sweepIndex;
    char                   scanString[RKMaximumStringLength];
    char                   response[RKMaximumStringLength];
    char                   dump[RKMaximumStringLength];
    pthread_t              threadId;

    // Status / health
    size_t                 memoryUsage;
    char                   statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t               statusBufferIndex;
    RKEngineState          state;
    uint64_t               tic;
    float                  lag;
};

RKSteerEngine *RKSteerEngineInit(void);
void RKSteerEngineFree(RKSteerEngine *);

void RKSteerEngineSetVerbose(RKSteerEngine *, const int);
void RKSteerEngineSetEssentials(RKSteerEngine *, const RKRadarDesc *,
                                RKPosition *, uint32_t *,
                                RKConfig *,   uint32_t *);
void RKSteerEngineSetScanRepeat(RKSteerEngine *, const bool);

int RKSteerEngineStart(RKSteerEngine *);
int RKSteerEngineStop(RKSteerEngine *);

// RKScanPath RKSteerEngineMakeScanPath(RKScanMode,
//                                      const float elevationStart, const float elevationEnd,
//                                      const float azimuthStart, const float azimuthEnd,
//                                      const float rate);

void RKSteerEngineStopSweeps(RKSteerEngine *);
void RKSteerEngineStartSweeps(RKSteerEngine *);
void RKSteerEngineClearSweeps(RKSteerEngine *);
void RKSteerEngineClearHole(RKSteerEngine *);
void RKSteerEngineClearDeck(RKSteerEngine *);
void RKSteerEngineNextHitter(RKSteerEngine *);
void RKSteerEngineArmSweeps(RKSteerEngine *, const RKScanRepeat);
int RKSteerEngineAddLineupSweep(RKSteerEngine *, const RKScanPath);
int RKSteerEngineAddPinchSweep(RKSteerEngine *, const RKScanPath);

// void RKSteerEngineUpdatePositionFlags(RKSteerEngine *, RKPosition *);

RKScanAction *RKSteerEngineGetActionV1(RKSteerEngine *, RKPosition *);
RKScanAction *RKSteerEngineGetAction(RKSteerEngine *, RKPosition *);

bool RKSteerEngineIsExecutable(const char *);
int RKSteerEngineExecuteString(RKSteerEngine *, const char *, char *);

size_t RKSteerEngineScanSummary(RKSteerEngine *, char *);

char *RKSteerEngineStatusString(RKSteerEngine *);

#endif
