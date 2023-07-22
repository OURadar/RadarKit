//
//  RKSteerEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//
//  Significant contributions from Ming-Duan Tze in 2023
//
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKSteerEngine.h>

#pragma mark - Internal Functions

static void RKSteerEngineUpdateStatusString(RKSteerEngine *engine) {
    char *string;
    RKScanObject *V = &engine->vcpHandle;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    uint32_t index = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
    RKPosition *position = &engine->positionBuffer[index];

    RKScanAction *action = &engine->actions[engine->actionIndex];

    sprintf(string, "%s%s%s %08d   %sEL%s %6.2f° @ %+5.1f°/s [%6.2f°]   %sAZ%s %6.2f @ %+5.1f°/s [%6.2f]   %s%s%s   %s%s%s",
        V->progress & RKScanProgressSetup  ? (rkGlobalParameters.showColor ? RKMonokaiOrange "S" RKNoColor : "S") : ".",
        V->progress & RKScanProgressMiddle ? "m" : ".",
        V->progress & RKScanProgressEnd    ? (rkGlobalParameters.showColor ? RKMonokaiGreen "E" RKNoColor : "E") : ".",
        V->tic,
        rkGlobalParameters.statusColor ? RKPositionElevationFlagColor(position->flag) : "",
        rkGlobalParameters.statusColor ? RKNoColor : "",
        position->elevationDegrees,
        position->elevationVelocityDegreesPerSecond,
        position->sweepElevationDegrees,
        rkGlobalParameters.statusColor ? RKPositionAzimuthFlagColor(position->flag) : "",
        rkGlobalParameters.statusColor ? RKNoColor : "",
        position->azimuthDegrees,
        position->azimuthVelocityDegreesPerSecond,
        position->sweepAzimuthDegrees,
        V->active && rkGlobalParameters.showColor ? RKDeepPinkColor : "",
        V->active ? RKScanModeString(V->batterScans[V->i].mode) : " - ",
        V->active && rkGlobalParameters.showColor ? RKNoColor : "",
        RKInstructIsNone(action->mode[0]) ? "" : RKMonokaiGreen,
        RKPedestalActionString(action),
        RKInstructIsNone(action->mode[0]) ? "" : RKNoColor);

    // RKLog(">%s %s\n", engine->name, string);

    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

// Should make the piece-wise thresholds [4] in pref.con
float RKSteerEngineGetRate(const float delta, RKPedestalAxis axis) {
    float rate = 0.0f;
    float gamma = fabsf(delta);
    if (axis == RKPedestalAxisAzimuth) {
        if (gamma >= 20.0f) {
            rate = 30.0f;
        } else if (gamma >= 10.0f) {
            rate = 20.0f;
        } else if (gamma >= 5.0f) {
            rate = 10.0f;
        } else if (gamma >= 1.5f) {
            rate = 3.0f;
        } else {
            rate = 1.0f;
        }
    } else if (axis == RKPedestalAxisElevation) {
        if (gamma >= 20.0f) {
            rate = 15.0f;
        } else if (gamma >= 3.0f) {
            rate = 10.0f;
        } else if (gamma >= 0.5f) {
            rate = 3.0f;
        } else {
            rate = 1.0f;
        }
    }
    return delta < 0.0f ? -rate : rate;
}

void RKSteerEngineStopSweeps(RKSteerEngine *engine) {
    engine->vcpHandle.progress = RKScanProgressStopPedestal;
    engine->vcpHandle.active = false;
}

void RKSteerEngineStartSweeps(RKSteerEngine *engine) {
    engine->vcpHandle.active = true;
}

void RKSteerEngineClearSweeps(RKSteerEngine *engine) {
    engine->vcpHandle.progress = RKScanProgressNone;
    engine->vcpHandle.inTheHoleCount = 0;
    engine->vcpHandle.onDeckCount = 0;
    engine->vcpHandle.sweepCount = 0;
    engine->vcpHandle.i = 0;
}

void RKSteerEngineClearHole(RKSteerEngine *engine) {
    engine->vcpHandle.inTheHoleCount = 0;
    engine->vcpHandle.onDeckCount = 0;
}

void RKSteerEngineClearDeck(RKSteerEngine *engine) {
    engine->vcpHandle.onDeckCount = 0;
}

void RKSteerEngineNextHitter(RKSteerEngine *engine) {
    RKScanObject *V = &engine->vcpHandle;
    memcpy(V->batterScans, V->onDeckScans, V->onDeckCount * sizeof(RKScanPath));
    memcpy(V->onDeckScans, V->inTheHoleScans, V->inTheHoleCount * sizeof(RKScanPath));
    V->sweepCount = V->onDeckCount;
    V->onDeckCount = V->inTheHoleCount;
    V->i = 0;
    V->active = true;
}

void RKSteerEngineArmSweeps(RKSteerEngine *engine, const RKScanRepeat repeat) {
    engine->vcpHandle.progress = RKScanProgressNone;
    if (repeat == RKScanRepeatForever) {
        RKSteerEngineSetScanRepeat(engine, true);
    } else if (engine->vcpHandle.option & RKScanOptionRepeat) {
        RKSteerEngineSetScanRepeat(engine, false);
    }
    engine->vcpHandle.i = 0;
    engine->vcpHandle.active = true;
}

int RKSteerEngineAddLineupSweep(RKSteerEngine *engine, const RKScanPath scan) {
    RKScanObject *V = &engine->vcpHandle;
    if (V->inTheHoleCount < RKMaximumScanCount) {
        V->inTheHoleScans[V->inTheHoleCount++] = scan;
        V->onDeckScans[V->onDeckCount++] = scan;
    } else {
        RKLog("%s Error. Cannot add more scans.  V->inTheHoleCount = %d\n", engine->name, V->inTheHoleCount);
        return RKResultTooBig;
    }
    return RKResultSuccess;
}

int RKSteerEngineAddPinchSweep(RKSteerEngine *engine, const RKScanPath scan) {
    RKScanObject *V = &engine->vcpHandle;
    if (V->onDeckCount < RKMaximumScanCount) {
        V->onDeckScans[V->onDeckCount++] = scan;
    } else {
        RKLog("%s Error. Cannot add more scans.  V->onDeckScans = %d\n", engine->name, V->onDeckScans);
        return RKResultTooBig;
    }
    return RKResultSuccess;
}

RKScanPath RKSteerEngineMakeScanPath(RKScanMode mode,
                                     const float elevationStart, const float elevationEnd,
                                     const float azimuthStart, const float azimuthEnd,
                                     const float rate) {
    RKScanPath sweep = {
        .mode = mode,
        .azimuthStart   = azimuthStart,
        .azimuthEnd     = azimuthEnd,
        .azimuthSlew    = rate,
        .elevationStart = elevationStart,
        .elevationEnd   = elevationEnd,
        .elevationSlew  = rate
    };
    return sweep;
}

int RKSteerEngineAddPPISet(RKSteerEngine *engine, const char *string, const bool once, char *response) {
    char args[4][256] = {"", "", "", ""};
    const int n = sscanf(string, "%*s %255s %255s %255s %255s", args[0], args[1], args[2], args[3]);

    float azimuthStart, azimuthEnd, elevationStart, elevationEnd, rate;

    if (response == NULL) {
        response = engine->response;
    }

    if (n < 2) {
        sprintf(response, "NAK. Ill-defined PPI array.   n = %d" RKEOL, n);
        return RKResultIncompleteScanDescription;
    }

    char *elevations = args[0];
    char *azimuth = args[1];

    if (n > 1) {
        azimuthStart = atof(azimuth);
    } else {
        azimuthStart = 0.0f;
    }
    azimuthEnd = azimuthStart;
    if (n > 2) {
        rate = atof(args[2]);
    } else {
        rate = RKDefaultScanSpeed;
    }

    int k = 0;
    char *token = strtok(elevations, ",");
    while (token != NULL && k++ < 100) {
        const int m = sscanf(token, "%f", &elevationStart);
        if (m == 0) {
            sprintf(response, "NAK. Ill-defined PPI component.   m = %d" RKEOL, m);
            return RKResultIncompleteScanDescription;
        }
        elevationEnd = elevationStart;

        RKScanPath scan = RKSteerEngineMakeScanPath(RKScanModePPI, elevationStart, elevationEnd, azimuthStart, azimuthEnd, rate);

        if (once) {
            RKSteerEngineAddPinchSweep(engine, scan);
        } else{
            RKSteerEngineAddLineupSweep(engine, scan);
        }
        token = strtok(NULL, ",");
    }

    return RKResultSuccess;
}

int RKSteerEngineAddRHISet(RKSteerEngine *engine, const char *string, const bool once, char *response) {
    char args[4][256] = {"", "", "", ""};
    const int n = sscanf(string, "%*s %255s %255s %255s %255s", args[0], args[1], args[2], args[3]);

    float azimuthStart, elevationStart, elevationEnd, rate;

    if (response == NULL) {
        response = engine->response;
    }

    if (n < 2) {
        sprintf(response, "NAK. Ill-defined RHI array.   n = %d" RKEOL, n);
        return RKResultIncompleteScanDescription;
    }

    char *elevations = args[0];
    char *azimuths = args[1];

    if (n > 2) {
        rate = atof(args[2]);
    } else {
        rate = RKDefaultScanSpeed;
    }

    int m = sscanf(elevations, "%f,%f", &elevationStart, &elevationEnd);
    if (m < 2) {
        sprintf(response, "NAK. Ill-defined RHI component.   m = %d" RKEOL, m);
        return RKResultIncompleteScanDescription;
    }

    bool flip = false;

    int k = 0;
    char *token = strtok(azimuths, ",");
    while (token != NULL && k++ < 180) {
        const int o = sscanf(token, "%f", &azimuthStart);
        if (o == 0) {
            sprintf(response, "NAK. Ill-defined RHI component.   o = %d" RKEOL, m);
            return RKResultIncompleteScanDescription;
            break;
        }

        RKScanPath scan = flip ?
            RKSteerEngineMakeScanPath(RKScanModeRHI, elevationEnd, elevationStart, azimuthStart, 0, -rate) :
            RKSteerEngineMakeScanPath(RKScanModeRHI, elevationStart, elevationEnd, azimuthStart, 0, rate);

        if (once) {
            RKSteerEngineAddPinchSweep(engine, scan);
        } else{
            RKSteerEngineAddLineupSweep(engine, scan);
        }
        token = strtok(NULL, ",");
        flip = !flip;
    }

    return RKResultSuccess;
}

#pragma mark - Delegate Workers

static void *steerer(void *_in) {
    RKSteerEngine *engine = (RKSteerEngine *)_in;

    int j, k, s;
    float theta, rate;

    RKPosition *new;
    RKPosition *old;

    struct timeval currentTime, triggerTime;
    struct timeval period = {.tv_sec = 2, .tv_usec = 0};

    // Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    RKLog("%s Started.   mem = %s B   positionIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->positionIndex);

    // Make vcpHandle seems like it has been forever since the last instruction
    engine->vcpHandle.tic = 99;

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    // Wait until there is something ingested
    s = 0;
    engine->state |= RKEngineStateSleep0;
    while (engine->tic < engine->radarDescription->positionBufferDepth / 8 && engine->state & RKEngineStateWantActive) {
        usleep(1000);
        if (++s % 200 == 0 && engine->verbose > 1) {
            RKLog("%s sleep 0/%.1f s\n", engine->name, (float)s * 0.001f);
        }
    }
    engine->state ^= RKEngineStateSleep0;
    engine->state |= RKEngineStateActive;

    timerclear(&currentTime);
    timerclear(&triggerTime);

    k = 0;
    while (engine->state & RKEngineStateWantActive) {
        // Wait until a position arrives
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->positionIndex &&  engine->state & RKEngineStateWantActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   positionIndex = %d\n",
                      engine->name, (float)s * 0.001f, k , *engine->positionIndex);
            }
        }
        engine->state ^= RKEngineStateSleep1;

        if (engine->tic % 10 == 0) {
            j = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
            new = &engine->positionBuffer[j];
            j = RKPreviousNModuloS(j, engine->radarDescription->positionBufferDepth / 8, engine->radarDescription->positionBufferDepth);
            old = &engine->positionBuffer[j];
            theta = new->timeDouble - old->timeDouble;
            rate = (float)(engine->radarDescription->positionBufferDepth / 8) / theta;
            s = (int)ceilf(RKPedestalActionPeriod * rate);
            if (engine->vcpHandle.toc != s) {
                engine->vcpHandle.toc = s;
                RKLog("%s Position rate = %.1f Hz   toc -> %d\n", engine->name, rate, s);
            }
        }

        gettimeofday(&currentTime, NULL);
        if (timercmp(&currentTime, &triggerTime, >=)) {
            timeradd(&currentTime, &period, &triggerTime);
            RKScanObject *V = &engine->vcpHandle;
            if (V->option & RKScanOptionVerbose) {
                if (V->active) {
                    fprintf(stderr, "                 -- [ VCP sweep %d / %d -- prog %d ] --\n",
                            V->i,
                            V->sweepCount,
                            V->progress);
                } else {
                    fprintf(stderr, "                 -- [ VCP inactive ] --\n");
                }
            }
        }

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
    }

    engine->state ^= RKEngineStateActive;

    return NULL;
}

// Implementations

#pragma mark - Life Cycle

RKSteerEngine *RKSteerEngineInit(void) {
    RKSteerEngine *engine = (RKSteerEngine *)malloc(sizeof(RKSteerEngine));
    memset(engine, 0, sizeof(RKSteerEngine));
    sprintf(engine->name, "%s<PositionSteerer>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorSteerEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    sprintf(engine->vcpHandle.name, "VCP");
    engine->vcpHandle.option = RKScanOptionRepeat;
    engine->vcpHandle.active = false;
    engine->vcpHandle.toc = 3;
    engine->memoryUsage = sizeof(RKSteerEngine);
    engine->state = RKEngineStateAllocated;
    return engine;
}

void RKSteerEngineFree(RKSteerEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKSteerEngineSetVerbose(RKSteerEngine *engine, const int verbose) {
    engine->verbose = verbose;
    if (verbose > 1) {
        engine->vcpHandle.option |= RKScanOptionVerbose;
    }
}

void RKSteerEngineSetInputOutputBuffers(RKSteerEngine *engine, const RKRadarDesc *desc,
                                        RKPosition *positionBuffer, uint32_t *positionIndex,
                                        RKConfig   *configBuffer,   uint32_t *configIndex) {
    engine->radarDescription    = (RKRadarDesc *)desc;
    engine->positionBuffer      = positionBuffer;
    engine->positionIndex       = positionIndex;
    engine->configBuffer        = configBuffer;
    engine->configIndex         = configIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKSteerEngineSetScanRepeat(RKSteerEngine *engine, const bool value) {
    if (value) {
        engine->vcpHandle.option |= RKScanOptionRepeat;
        return;
    }
    if (engine->vcpHandle.option & RKScanOptionRepeat) {
        engine->vcpHandle.option ^= RKScanOptionRepeat;
    }
}

#pragma mark - Interactions

int RKSteerEngineStart(RKSteerEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating | RKEngineStateWantActive;
    if (pthread_create(&engine->threadId, NULL, steerer, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartRingPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKSteerEngineStop(RKSteerEngine *engine) {
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
    if (engine->threadId) {
        pthread_join(engine->threadId, NULL);
        engine->threadId = (pthread_t)0;
    } else {
        RKLog("%s Error. Invalid thread ID = %p\n", engine->name, engine->threadId);
    }
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

void RKSteerEngineUpdatePositionFlags(RKSteerEngine *engine, RKPosition *position) {
    RKScanObject *V = &engine->vcpHandle;
    if (V->active) {
        const RKScanPath *scan = &V->batterScans[V->i];
        position->flag |= RKPositionFlagVCPActive;
        // Sweep is active: data collection portion (i.e., exclude non-RKScanProgressMiddle)
        if (V->progress & RKScanProgressMiddle) {
            position->flag |= RKPositionFlagScanActive;
        }
        // Point / sweep flag
        switch (scan->mode) {
            case RKScanModePPI:
            case RKScanModeSector:
                position->flag |= RKPositionFlagAzimuthSweep;
                position->flag |= RKPositionFlagElevationPoint;
                break;
            case RKScanModeRHI:
                position->flag |= RKPositionFlagAzimuthPoint;
                position->flag |= RKPositionFlagElevationSweep;
                break;
            case RKScanModePoint:
                position->flag |= RKPositionFlagAzimuthPoint | RKPositionFlagElevationPoint;
                break;
            default:
                break;
        }
        // Completion flag
        if (V->progress & RKScanProgressEnd) {
            switch (scan->mode) {
                case RKScanModePPI:
                case RKScanModeSector:
                    position->flag |= RKPositionFlagAzimuthComplete;
                    break;
                case RKScanModeRHI:
                    position->flag |= RKPositionFlagElevationComplete;
                    break;
                case RKScanModePoint:
                    position->flag |= RKPositionFlagAzimuthComplete | RKPositionFlagElevationComplete;
                    break;
                default:
                    break;
            }
        }
        position->sweepElevationDegrees = scan->elevationStart;
        position->sweepAzimuthDegrees = scan->azimuthStart;
    }
}

RKScanAction *RKSteerEngineGetAction(RKSteerEngine *engine, RKPosition *pos) {
    RKScanAction *action = &engine->actions[engine->actionIndex];
    memset(action, 0, sizeof(RKScanAction));

    int a = 0;

    engine->tic++;

    RKScanObject *V = &engine->vcpHandle;

    const bool verbose = V->option & RKScanOptionVerbose;

    float del = 0.0f;
    float daz = 0.0f;
    float udel = 0.0f;
    float udaz = 0.0f;

    bool cross = false;

    if (V->active == false || V->sweepCount == 0) {
        if (V->progress & RKScanProgressStopPedestal) {
            RKLog("%s Stopping pedestal ...\n", engine->name);
            if (fabs(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                action->value[a] = 0.0f;
                V->tic = 0;
                a++;
            }
            if (fabs(pos->azimuthVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                action->value[a] = 0.0f;
                V->tic = 0;
            }
            V->progress ^= RKScanProgressStopPedestal;
            V->progress = RKScanProgressNone;
        }

        RKSteerEngineUpdateStatusString(engine);

        engine->actionIndex = RKNextModuloS(engine->actionIndex, RKPedestalActionBufferDepth);
        V->elevationPrevious = pos->elevationDegrees;
        V->azimuthPrevious = pos->azimuthDegrees;
        V->tic++;

        return action;
    }

    // if (V->progress == RKScanProgressNone) {
    //     ... do some basic setup, immediately go to next
    //     ... V->progress = RKScanProgressSetup;
    //     ... note: one position payload is used
    // } else if (V->progress == RKScanProgressSetup) {
    //     ... do some logic to determine:
    //     ... V->progress = RKScanProgressMiddle;
    // }
    //
    // if (V->progress & RKScanProgressMiddle) {
    //     ... do some logic to determine:
    //     ... V->progress |= RKScanProgressEnd;
    // }
    //
    // if (V->progress & RKScanProgressEnd) {
    //     V->progress ^= RKScanProgressEnd;
    //     ... get the next scan path, jump to RKScanProgressSetup
    // }

    const RKScanPath *scan = &V->batterScans[V->i];

    if (V->progress == RKScanProgressNone) {
        V->progress = RKScanProgressSetup;
        switch (scan->mode) {
            case RKScanModePPI:
                V->progress |= RKScanProgressMiddle;
            break;
        }
    } else if (V->progress & RKScanProgressSetup && V->tic >= V->toc) {
        switch (scan->mode) {
            case RKScanModeRHI:
            case RKScanModeSpeedDown:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                daz = RKMinDiff(scan->azimuthStart, pos->azimuthDegrees);
                udel = fabs(del);
                udaz = fabs(daz);
                if (udel >= RKPedestalPositionTolerance) {
                    if (engine->vcpHandle.option & RKScanOptionUsePoint) {
                        action->mode[a] = RKPedestalInstructTypeModePoint | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = scan->elevationStart;
                    } else {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                    }
                    V->tic = 0;
                    a++;
                } else if (fabsf(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                    a++;
                }
                if (udaz >= RKPedestalPositionTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->value[a] = RKSteerEngineGetRate(daz, RKPedestalAxisAzimuth);
                    V->tic = 0;
                } else if (fabsf(pos->azimuthVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                }
                if (udel < RKPedestalPositionTolerance && udaz < RKPedestalPositionTolerance) {
                    action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                    action->value[0] = scan->elevationSlew;
                    V->tic = 0;
                    if (verbose) {
                        RKLog("%s Info. Ready for RHI sweep %d - AZ %.2f° @ crossover EL %.2f°\n", engine->name,
                            V->i, scan->azimuthStart, scan->elevationEnd);
                    }
                    V->progress ^= RKScanProgressSetup;
                    V->progress |= RKScanProgressMiddle;
                }
                break;
            case RKScanModePPI:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                udel = fabs(del);
                if (udel >= RKPedestalPositionTolerance) {
                    if (engine->vcpHandle.option & RKScanOptionUsePoint) {
                        action->mode[a] = RKPedestalInstructTypeModePoint | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = scan->elevationStart;
                    } else {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                    }
                    if (verbose) {
                        RKLog("%s Info. V->i = %d  EL %.1f @ %.1f -> %.1f  del = %.1f  action->value[%d] = %.1f", engine->name,
                            V->i, pos->elevationDegrees, pos->elevationVelocityDegreesPerSecond, scan->elevationStart, del, a, action->value[a]);
                    }
                    V->tic = 0;
                    a++;
                } else if (fabs(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                    a++;
                }
                if (fabsf(scan->azimuthSlew - pos->azimuthVelocityDegreesPerSecond) >= RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->value[a] = scan->azimuthSlew;
                    V->tic = 0;
                }
                if (udel < RKPedestalPositionTolerance) {
                    if (verbose) {
                        RKLog("%s Info. Ready for PPI sweep %d - EL %.2f° @ crossover AZ %.2f°\n", engine->name,
                            V->i, scan->elevationStart, scan->azimuthEnd);
                    }
                    V->progress ^= RKScanProgressSetup;
                    V->progress |= RKScanProgressMiddle;
                }
                break;
            case RKScanModeSector:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                daz = RKMinDiff(scan->azimuthStart, pos->azimuthDegrees);
                udel = fabs(del);
                udaz = fabs(daz);
                if (udel >= RKPedestalPositionTolerance) {
                    if (engine->vcpHandle.option & RKScanOptionUsePoint) {
                        action->mode[a] = RKPedestalInstructTypeModePoint | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = scan->elevationStart;
                    } else {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                    }
                    V->tic = 0;
                    a++;
                } else if (fabsf(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                    a++;
                }
                if (udaz >= RKPedestalPositionTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->value[a] = RKSteerEngineGetRate(daz, RKPedestalAxisAzimuth);
                    V->tic = 0;
                } else if (fabsf(pos->azimuthVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                }
                if (udel < RKPedestalPositionTolerance && udaz < RKPedestalPositionTolerance) {
                    action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->value[0] = scan->azimuthSlew;
                    V->tic = 0;
                    if (verbose) {
                        RKLog("%s Info. Ready for SEC sweep %d - EL %.1f° @ crossover AZ %.1f°\n", engine->name,
                            V->i, scan->elevationStart, scan->azimuthEnd);
                    }
                    V->progress ^= RKScanProgressSetup;
                    V->progress |= RKScanProgressMiddle;
                }
                break;
            case RKScanModePoint:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                daz = RKMinDiff(scan->azimuthStart, pos->azimuthDegrees);
                udel = fabs(del);
                udaz = fabs(daz);
                if (udel >= RKPedestalPositionTolerance) {
                    if (engine->vcpHandle.option & RKScanOptionUsePoint) {
                        action->mode[a] = RKPedestalInstructTypeModePoint | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = scan->elevationStart;
                    } else {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->value[a] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                    }
                    V->tic = 0;
                    a++;
                } else if (fabsf(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                    a++;
                }
                if (udaz >= RKPedestalPositionTolerance) {
                    if (engine->vcpHandle.option & RKScanOptionUsePoint) {
                        action->mode[a] = RKPedestalInstructTypeModePoint | RKPedestalInstructTypeAxisAzimuth;
                        action->value[a] = scan->azimuthStart;
                    } else {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                        action->value[a] = RKSteerEngineGetRate(daz, RKPedestalAxisAzimuth);
                    }
                    V->tic = 0;
                } else if (fabsf(pos->azimuthVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    action->value[a] = 0.0f;
                    V->tic = 0;
                }
                if (udel < RKPedestalPositionTolerance && udaz < RKPedestalPositionTolerance) {
                    if (verbose) {
                        RKLog("%s Info. Pointing (EL %.2f°, AZ %.2f°)\n", engine->name,
                            scan->elevationStart, scan->azimuthStart);
                    }
                    V->progress ^= RKScanProgressSetup;
                    V->progress |= RKScanProgressEnd;
                }
                break;
            default:
                break;
        }
    } // if (V->progress == ...

    if (V->progress & RKScanProgressMiddle) {
        switch (scan->mode) {
            case RKScanModeRHI:
                // If the action was just sent, pedestal has not yet accelerated
                if (fabsf(pos->elevationVelocityDegreesPerSecond) <= RKPedestalVelocityTolerance) {
                    break;
                }
                cross = RKAngularCrossOver(pos->elevationDegrees, V->elevationPrevious, scan->elevationEnd);
                if (cross) {
                    V->progress ^= RKScanProgressMiddle;
                    V->progress |= RKScanProgressEnd;
                    if (verbose) {
                        RKLog("%s End crossover detected @ EL %.2f° [%.2f° -> %.2f°]\n", engine->name,
                            scan->elevationEnd, V->elevationPrevious, pos->elevationDegrees);
                    }
                }
                break;
            case RKScanModePPI:
                // If the action was just sent, pedestal has not yet accelerated
                if (fabsf(pos->azimuthVelocityDegreesPerSecond) <= RKPedestalVelocityTolerance) {
                    break;
                }
                // Send the stop action again if the elevation continues to move
                if (!(V->progress & RKScanProgressSetup) && V->tic > 25 && fabs(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    RKLog("%s Issuing another EL standby action for sweep %.1f ... EL %.1f @ %.1f\n", engine->name,
                        scan->elevationStart, pos->elevationDegrees, pos->elevationVelocityDegreesPerSecond);
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->value[0] = 0.0f;
                    V->tic = 0;
                }
                // Check for a cross-over trigger
                cross = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, scan->azimuthEnd);
                if (cross) {
                    V->progress |= RKScanProgressEnd;
                    if (verbose) {
                        RKLog("%s End crossover detected @ AZ %.2f° [%.2f° -> %.2f°]\n", engine->name,
                            scan->azimuthEnd, V->azimuthPrevious, pos->azimuthDegrees);
                    }
                }
                break;
            case RKScanModeSector:
                // If the action was just sent, pedestal has not yet accelerated
                if (fabsf(pos->azimuthVelocityDegreesPerSecond) <= RKPedestalVelocityTolerance) {
                    break;
                }
                cross = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, scan->azimuthEnd);
                if (cross) {
                    V->progress |= RKScanProgressEnd;
                    if (verbose) {
                        RKLog("%s End crossover detected @ AZ %.2f° [%.2f° -> %.2f°]\n", engine->name,
                            scan->azimuthEnd, V->azimuthPrevious, pos->azimuthDegrees);
                    }
                }
                break;
            case RKScanModePoint:
                RKLog("%s Info. Point should not pass here.\n", engine->name);
                V->progress ^= RKScanProgressMiddle;
                V->progress |= RKScanProgressEnd;
            default:
                break;
        }
    }

    RKSteerEngineUpdatePositionFlags(engine, pos);

    RKSteerEngineUpdateStatusString(engine);

    // Sweep ended, go to the next sweep
    if (V->progress & RKScanProgressEnd) {
        V->progress ^= RKScanProgressEnd;
        V->i = RKNextModuloS(V->i, V->sweepCount);
        if (V->i == 0) {
            if (V->option & RKScanOptionRepeat) {
                RKSteerEngineNextHitter(engine);
                RKLog("%s VCP repeats.\n", engine->name);
            } else {
                RKLog("%s VCP stops.\n", engine->name);
                if (V->sweepCount == 1 && V->batterScans[0].mode == RKScanModePoint) {
                    RKSteerEngineClearSweeps(engine);
                }
                V->active = false;
            }
        }
        V->progress |= RKScanProgressSetup;
    }

    engine->actionIndex = RKNextModuloS(engine->actionIndex, RKPedestalActionBufferDepth);
    V->elevationPrevious = pos->elevationDegrees;
    V->azimuthPrevious = pos->azimuthDegrees;
    V->tic++;

    return action;
}

RKSteerCommand RKSteerCommandFromString(const char *string) {
    char token[64];
    int s = sscanf(string, "%63s %*s", token);
    RKSteerCommand type = RKSteerCommandNone;
    if (s == 0) {
        return type;
    } else if (!strcmp(token, "home")) {
        type = RKSteerCommandHome | RKSteerCommandImmediate;
    } else if (!strcmp(token, "point")) {
        type = RKSteerCommandPoint | RKSteerCommandImmediate;
    } else if (!strcmp(token, "pp")) {
        type = RKSteerCommandPPISet;
    } else if (!strcmp(token, "ipp")) {
        type = RKSteerCommandPPISet | RKSteerCommandImmediate;
    } else if (!strcmp(token, "opp")) {
        type = RKSteerCommandPPISet | RKSteerCommandOnce;
    } else if (!strcmp(token, "rr")) {
        type = RKSteerCommandRHISet;
    } else if (!strcmp(token, "irr")) {
        type = RKSteerCommandRHISet | RKSteerCommandImmediate;
    } else if (!strcmp(token, "orr")) {
        type = RKSteerCommandRHISet | RKSteerCommandOnce;
    } else if (!strcmp(token, "vol")) {
        type = RKSteerCommandVolume;
    } else if (!strcmp(token, "ivol")) {
        type = RKSteerCommandVolume | RKSteerCommandImmediate;
    } else if (!strcmp(token, "ovol")) {
        type = RKSteerCommandVolume | RKSteerCommandOnce;
    } else if (!strcmp(token, "summ") || !strncmp(token, "stat", 4)) { // stat, status, state
        type = RKSteerCommandSummary;
    } else if (!strcmp(token, "start") || !strcmp(token, "run") || !strcmp(token, "go")) {
        type = RKSteerCommandScanStart;
    } else if (!strcmp(token, "stop") || !strcmp(token, "end")) {
        type = RKSteerCommandScanStop;
    } else if (!strcmp(token, "next")) {
        type = RKSteerCommandScanNext;
    }
    return type;
}

bool RKSteerEngineIsExecutable(const char *string) {
    return (RKSteerCommandFromString(string) & RKSteerCommandInstructionMask) != RKSteerCommandNone;
}

int RKSteerEngineExecuteString(RKSteerEngine *engine, const char *string, char _Nullable *response) {
    const RKSteerCommand command = RKSteerCommandFromString(string);
    const bool immediatelyDo = RKSteerCommandIsImmediate(command);
    const bool onlyOnce = RKSteerCommandIsOnce(command);

    if (response == NULL) {
        response = engine->response;
    }

    // RKLog("%s Interpreted %s\n", engine->name, RKVariableInString("command", &command, RKValueTypeInt32InHex));

    int s;
    int result;
    float azimuthStart, azimuthEnd, elevationStart, elevationEnd, rate;

    bool valid = true;
    char symbol[4] = "";
    char args[4][256] = {"", "", "", ""};
    RKSteerCommand motion = command & RKSteerCommandInstructionMask;

    switch (motion) {
        case RKSteerCommandSummary:
            s = sprintf(response, "ACK. Volume summary retrieved.\n\n");
            s += RKSteerEngineScanSummary(engine, response + s);
            sprintf(response + s, "%s   %s   %s   %s" RKEOL,
                RKVariableInString("active", &engine->vcpHandle.active, RKValueTypeBool),
                RKVariableInString("sweepCount", &engine->vcpHandle.sweepCount, RKValueTypeUInt16),
                RKVariableInString("onDeckCount", &engine->vcpHandle.onDeckCount, RKValueTypeUInt16),
                RKVariableInString("inTheHoleCount", &engine->vcpHandle.inTheHoleCount, RKValueTypeUInt16));
            return RKResultSuccess;
            break;
        case RKSteerCommandScanStart:
            RKSteerEngineArmSweeps(engine, RKScanRepeatForever);
            s = sprintf(response, "ACK. Volume starts.\n\n");
            s += RKSteerEngineScanSummary(engine, response + s);
            sprintf(response + s - 1, RKEOL);
            return RKResultSuccess;
            break;
        case RKSteerCommandScanStop:
            RKSteerEngineStopSweeps(engine);
            sprintf(response, "ACK. Volume stopped." RKEOL);
            return RKResultSuccess;
            break;
        case RKSteerCommandScanNext:
            RKSteerEngineNextHitter(engine);
            sprintf(response, "ACK. Volume advanced." RKEOL);
            break;
        case RKSteerCommandNone:
            sprintf(response, "NAK. Command '%s' not understood. Ask my father." RKEOL, string);
            return RKResultFailedToExecuteCommand;
            break;
        default:
            break;
    }

    // The rest of the function assumes a scan / volume will be created
    if (RKSteerCommandIsMotion(command)) {
        if (motion == RKSteerCommandPoint) {
            RKSteerEngineSetScanRepeat(engine, false);
        } else {
            RKSteerEngineSetScanRepeat(engine, true);
        }
    } else {
        sprintf(response, "NAK. Command '%s' not understood. Ask my father." RKEOL, string);
        RKLog("%s Non-motion commands should not be here.\n", engine->name);
        return RKResultFailedToExecuteCommand;
    }

    if (immediatelyDo) {
        RKSteerEngineClearSweeps(engine);
    } else if (onlyOnce) {
        RKSteerEngineClearDeck(engine);
    } else {
        RKSteerEngineClearHole(engine);
    }

    if (motion == RKSteerCommandPPISet) {

        // pp 2,4,6,8,10 45 18 - PPI at elevations [2, 4, 6, 8, 10] degs, azimuth 45, speed 18 deg/s
        result = RKSteerEngineAddPPISet(engine, string, onlyOnce, response);
        if (result != RKResultSuccess) {
            return result;
        }

    } else if (motion == RKSteerCommandRHISet) {

        // rr 0,20 10,20,30 10 - RHI at elevations [0, 20] degs, azimuths [10, 20, 30], speed 10 deg/s
        result = RKSteerEngineAddRHISet(engine, string, onlyOnce, response);
        if (result != RKResultSuccess) {
            return result;
        }

    } else if (motion == RKSteerCommandVolume) {

        // vol p 2 45 18/p 4 45 18/p 6 45 18/p 8 45 18/p 10 45 18/p 12 45 18 - PPI set like the pp example
        int k = 0;
        char *saved;
        char *token = strchr(string, ' ');
        do {
            token++;
        } while (*token == ' ' && k++ < 8);
        if (*token == ' ') {
            RKLog("%s Unxpected vol command string\n", engine->name);
            return RKResultFailedToSetVCP;
        }
        strncpy(engine->scanString, token, RKMaximumStringLength - 1);

        RKScanMode mode = RKScanModeNone;

        k = 0;
        token = strtok_r(engine->scanString, "/", &saved);
        while (token != NULL && k++ < 100) {
            // parse in the usual way
            const int m = sscanf(token, "%3s %255s %255s %255s", symbol, args[0], args[1], args[2]);
            if (m < 2) {
                sprintf(response, "NAK. Ill-defined volume component.   m = %d" RKEOL, m);
                valid = false;
                break;
            }
            if (strlen(symbol) == 1) {
                if (*symbol == 'p') {
                    mode = RKScanModePPI;
                } else if (*symbol == 'r') {
                    mode = RKScanModeRHI;
                } else if (*symbol == 's') {
                    mode = RKScanModeSector;
                } else if (*symbol == 'b') {
                    mode = RKScanModeSpeedDown;
                } else {
                    RKLog("%s Error. Scan mode '%s' not understood.\n", engine->name, symbol);
                    valid = false;
                    break;
                }
                const int o = sscanf(args[0], "%f,%f", &elevationStart, &elevationEnd);
                if (o < 1) {
                    sprintf(response, "NAK. Ill-defined volume component.   o = %d" RKEOL, o);
                    valid = false;
                    break;
                } else if (o == 1) {
                    elevationEnd = elevationStart;
                }
                const int p = sscanf(args[1], "%f,%f", &azimuthStart, &azimuthEnd);
                if (p < 1) {
                    sprintf(response, "NAK. Ill-defined volume component.   p = %d" RKEOL, o);
                    valid = false;
                    break;
                } else if (p == 1) {
                    azimuthEnd = azimuthStart;
                }
                rate = atof(args[2]);

                RKScanPath scan = RKSteerEngineMakeScanPath(mode, elevationStart, elevationEnd, azimuthStart, azimuthEnd, rate);

                if (onlyOnce) {
                    RKSteerEngineAddPinchSweep(engine, scan);
                } else {
                    RKSteerEngineAddLineupSweep(engine, scan);
                }
            } else if (!strcmp("pp", symbol)) {
                result = RKSteerEngineAddPPISet(engine, token, onlyOnce, response);
                valid = result == RKResultSuccess;
            } else if (!strcmp("rr", symbol)) {
                result = RKSteerEngineAddRHISet(engine, token, onlyOnce, response);
                valid = result == RKResultSuccess;
            } else if (!strcmp("ss", symbol)) {
                result = RKResultFailedToExecuteCommand;
                valid = false;
            }
            token = strtok_r(NULL, "/", &saved);
        }

    } else if (motion == RKSteerCommandPoint) {

        const int n = sscanf(string, "%*s %16s %16s", args[0], args[1]);
        const float elevation = CLAMP(atof(args[0]), -2.0f, 182.0f);
        const float azimuth = CLAMP(atof(args[1]), 0.0f, 360.0f);
        if (n != 2) {
            RKLog("%s Expected two arguments for point.   n = %d\n", engine->name, n);
        }
        RKScanPath scan = RKSteerEngineMakeScanPath(RKScanModePoint, elevation, elevation, azimuth, azimuth, NAN);
        RKSteerEngineAddPinchSweep(engine, scan);

    } else if (motion == RKSteerCommandHome) {

        const float elevation = 0.0f;
        const float azimuth = engine->radarDescription->heading;
        RKScanPath scan = RKSteerEngineMakeScanPath(RKScanModePoint, elevation, elevation, azimuth, azimuth, NAN);
        RKSteerEngineAddPinchSweep(engine, scan);

    } else {

        sprintf(response, "NAK. Nothing. Ask my father." RKEOL);
        return RKResultFailedToSetVCP;

    }

    if (valid) {
        size_t s = sprintf(response, "ACK. Volume created.\n\n");

        char *summary = response + s;

        s += RKSteerEngineScanSummary(engine, response + s);
        RKSteerEngineStartSweeps(engine);
        if (immediatelyDo || engine->vcpHandle.sweepCount == 0) {
            RKSteerEngineNextHitter(engine);
        }

        int k = sprintf(engine->dump, "New volume\n");
        RKIndentCopy(engine->dump + k, summary, 31);
        RKStripTail(engine->dump + k);
        RKLog("%s %s\n", engine->name, engine->dump);

        if (immediatelyDo) {
            sprintf(response + s, "ACK. Volume in effect." RKEOL);
        } else {
            sprintf(response + s, "ACK. Volume in queue." RKEOL);
        }
    } else {
        return RKResultFailedToSetVCP;
    }

    return RKResultSuccess;
}

static char *scanModeString(const RKScanMode mode) {
    switch (mode) {
        case RKScanModePPI:
            return "PPI";
        case RKScanModeRHI:
            return "RHI";
        case RKScanModeSector:
            return "SEC";
        case RKScanModePoint:
            return "SPT";
        default:
            return "UNK";
    }
    return "NAN";
}

static size_t makeSweepMessage(RKScanPath *scanPaths, char *string, const int count, const RKScanHitter linetag) {
    size_t k = 0;
    char prefix[8];
    switch (linetag){
        case RKScanAtBat:
            strncpy(prefix, "      ", 8);
            break;
        case RKScanPinch:
            strncpy(prefix, "PINCH ", 8);
            break;
        case RKScanLine:
            strncpy(prefix, "LINEUP", 8);
            break;
        default:
            break;
    }

    // First pass, get the maximum column width necessary for E and A
    char format[80];
    int es = 0;
    int ee = 0;
    int ev = 0;
    int as = 0;
    int ae = 0;
    int av = 0;
    const int n = 1;
    for (int i = 0; i < count; i++) {
        // int w1 = sprintf(format, "%.1f", scanPaths[i].azimuthSlew);
        // int w2 = RKDigitWidth(scanPaths[i].azimuthSlew, 1);
        // printf("w1 = %d   w2 = %d\n", w1, w2);
        es = MAX(es, RKDigitWidth(scanPaths[i].elevationStart, n));
        ee = MAX(es, RKDigitWidth(scanPaths[i].elevationEnd, n));
        ev = MAX(ev, RKDigitWidth(scanPaths[i].elevationSlew, n));
        as = MAX(as, RKDigitWidth(scanPaths[i].azimuthStart, n));
        ae = MAX(ae, RKDigitWidth(scanPaths[i].azimuthEnd, n));
        av = MAX(av, RKDigitWidth(scanPaths[i].azimuthSlew, n));
    }

    // Second pass, actually print it out using the consitent column width
    for (int i = 0; i < count; i++) {
        k += sprintf(string + k, "%-7s %d : %s%s%s",
                     prefix,
                     i,
                     rkGlobalParameters.statusColor ? RKMonokaiPink : "",
                     scanModeString(scanPaths[i].mode),
                     rkGlobalParameters.statusColor ? RKNoColor : "");
        switch (scanPaths[i].mode) {
            case RKScanModePPI:
                sprintf(format, " EL %%%d.1f°   AZ %%%d.1f°   @ %%%d.1f°/s\n", es, ae, av);
                k += sprintf(string + k, format,
                             scanPaths[i].elevationStart,
                             scanPaths[i].azimuthEnd,
                             scanPaths[i].azimuthSlew);
                break;
            case RKScanModeRHI:
                sprintf(format, " AZ %%%d.1f°   EZ %%%d.1f°-%%%d.1f°   @ %%%d.1f°/s\n", as, es, ee, ev);
                k += sprintf(string + k, format,
                             scanPaths[i].azimuthStart,
                             scanPaths[i].elevationStart,
                             scanPaths[i].elevationEnd,
                             scanPaths[i].elevationSlew);
                break;
            case RKScanModeSector:
                sprintf(format, " EL %%%d.1f°   AZ %%%d.1f°-%%%d.1f°   @ %%%d.1f°/s\n", es, as, ae, av);
                k += sprintf(string + k, format,
                             scanPaths[i].elevationStart,
                             scanPaths[i].azimuthStart,
                             scanPaths[i].azimuthEnd,
                             scanPaths[i].azimuthSlew);
                break;
            case RKScanModePoint:
                k += sprintf(string + k, " EL %.1f°   AZ %.1f°\n",
                             scanPaths[i].elevationStart,
                             scanPaths[i].azimuthStart);
                break;
            default:
                k += sprintf(string + k, " EL %.1f° / %.1f°   AZ %.1f° / %.1f°\n",
                             scanPaths[i].elevationStart,
                             scanPaths[i].elevationEnd,
                             scanPaths[i].azimuthStart,
                             scanPaths[i].azimuthEnd);
                break;
        }
    }
    return k;
}


size_t RKSteerEngineScanSummary(RKSteerEngine *engine, char *string) {
    RKScanObject *V = &engine->vcpHandle;
    size_t s = makeSweepMessage(V->batterScans, string, V->sweepCount, RKScanAtBat);
    if (V->onDeckCount != V->inTheHoleCount || memcmp(V->onDeckScans, V->inTheHoleScans, RKMaximumScanCount * sizeof(RKScanPath))) {
        s += makeSweepMessage(V->onDeckScans, string + s, V->onDeckCount, RKScanPinch);
    }
    if (V->option & RKScanOptionRepeat) {
        s += makeSweepMessage(V->inTheHoleScans, string + s, V->inTheHoleCount, RKScanLine);
    }
    if (V->sweepCount == 0 && V->onDeckCount == 0 && V->inTheHoleCount == 0) {
        s = sprintf(string, "(empty)\n");
    }
    return s;
}

char *RKSteerEngineStatusString(RKSteerEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
