//
//  RKSteerEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
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

    sprintf(string, "%s%s%s%s  T%3d  EL%5.2f @ %5.1f °/s   AZ%7.2f @ %5.1f °/s   M%d '%s%s%s'",
        V->progress & RKScanProgressSetup  ? (rkGlobalParameters.showColor ? RKMonokaiOrange "S" RKNoColor : "S") : ".",
        V->progress & RKScanProgressMiddle ? "m" : ".",
        V->progress & RKScanProgressMarker ? (rkGlobalParameters.showColor ? RKMonokaiGreen "M" RKNoColor : "M") : ".",
        V->progress & RKScanProgressEnd    ? (rkGlobalParameters.showColor ? RKMonokaiGreen "E" RKNoColor : "E") : ".",
        V->tic,
        position->elevationDegrees, position->elevationVelocityDegreesPerSecond,
        position->azimuthDegrees, position->azimuthVelocityDegreesPerSecond,
        V->batterScans[V->i].mode,
        RKInstructIsNone(action->mode[0]) ? "" : RKMonokaiGreen,
        RKPedestalActionString(action),
        RKInstructIsNone(action->mode[0]) ? "" : RKNoColor);

    // RKLog(">%s %s\n", engine->name, string);

    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

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
        } else if (gamma >= 2.0f) {
            rate = 3.0f;
        } else {
            rate = 1.0f;
        }
    } else if (axis == RKPedestalAxisElevation) {
        if (gamma >= 20.0f) {
            rate = 15.0f;
        } else if (gamma >= 7.0f) {
            rate = 10.0f;
        } else if (gamma >= 2.0f) {
            rate = 3.0f;
        } else {
            rate = 1.0f;
        }
    }
    return delta < 0.0f ? -rate : rate;
}

void RKSteerEngineStopSweeps(RKSteerEngine *engine) {
    engine->vcpHandle.active = false;
}

void RKSteerEngineClearSweeps(RKSteerEngine *engine) {
    engine->vcpHandle.progress = RKScanProgressNone;
    engine->vcpHandle.sweepCount = 0;
    engine->vcpHandle.onDeckCount = 0;
    engine->vcpHandle.inTheHoleCount = 0;
    engine->vcpHandle.i = 0;
    engine->vcpHandle.j = 0;
    engine->vcpHandle.active = true;
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
    memcpy(V->onDeckScans, V->inTheHoleScans, (V->onDeckCount + V->inTheHoleCount) * sizeof(RKScanPath));
    V->sweepCount = V->onDeckCount;
    V->onDeckCount = V->inTheHoleCount;
    V->i = 0;
    V->j = 0;
    V->active = true;
}

void RKSteerEngineArmSweeps(RKSteerEngine *engine, const RKScanRepeat repeat) {
    engine->vcpHandle.progress = RKScanProgressNone;
    if (repeat == RKScanRepeatForever) {
        engine->vcpHandle.option |= RKScanOptionRepeat;
    }
    engine->vcpHandle.i = 0;
    engine->vcpHandle.j = 0;
    engine->vcpHandle.active = true;
}

int RKSteerEngineAddLineupSweep(RKSteerEngine *engine, const RKScanPath scan) {
    RKScanObject *V = &engine->vcpHandle;
    if (V->inTheHoleCount < RKMaximumScanCount - 1) {
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
    if (V->onDeckCount < RKMaximumScanCount - 1) {
        V->onDeckScans[V->onDeckCount++] = scan;
    } else {
        RKLog("%s Error. Cannot add more scans.  V->inTheHoleCount = %d\n", engine->name, V->inTheHoleCount);
        return RKResultTooBig;
    }
    return RKResultSuccess;
}

// RKScanAction *RKPedestal

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
        // Sweep is active: data collection portion (i.e., exclude transitions)
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
            default:
                position->flag |= RKPositionFlagAzimuthPoint | RKPositionFlagElevationPoint;
                break;
        }
        // Completion flag
        if (V->progress & RKScanProgressMarker) {
            switch (scan->mode) {
                case RKScanModePPI:
                case RKScanModeSector:
                    position->flag |= RKPositionFlagAzimuthComplete;
                    break;
                case RKScanModeRHI:
                    position->flag |= RKPositionFlagElevationComplete;
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

    engine->tic++;

    RKScanObject *V = &engine->vcpHandle;

    if (V->sweepCount == 0 || V->active == false) {
        return action;
    }

    const bool verbose = V->option & RKScanOptionVerbose;

    float del = 0.0f;
    float daz = 0.0f;
    float udel = 0.0f;
    float udaz = 0.0f;

    bool cross = false;

    // if (V->progress == RKScanProgressNone) {
    //     ... do some basic setup, immediately go to next
    //     ... note: one position data is used
    //     ... V->progress = RKScanProgressSetup;
    // } else if (V->progress == RKScanProgressSetup) {
    //     ... do some logic to determine:
    //     ... V->progress = RKScanProgressMiddle;
    // }
    //
    // if (V->progress & RKScanProgressMiddle) {
    //     ... do some logic to determine:
    //     ... V->progress |= RKScanProgressMarker;
    // }
    //
    // if (V->progress & RKScanProgressMarker) {
    //     V->progress ^= RKScanProgressMarker;
    //     ... for consumer to know a sweep cross the marker
    // }
    //
    // if (V->progress & RKScanProgressEnd) {
    //     V->progress ^= RKScanProgressEnd;
    //     ... get the next scan path, jump to RKScanProgressSetup
    // }

    int a = 0;
    const RKScanPath *scan = &V->batterScans[V->i];

    if (V->progress == RKScanProgressNone) {
        V->progress = RKScanProgressSetup;
        switch (scan->mode) {
            case RKScanModePPI:
                V->progress |= RKScanProgressMiddle;
            break;
        }
    } else if (V->progress & RKScanProgressSetup && V->tic >= V->toc) {
        // RKLog("%s %s   %s\n", engine->name,
        //     RKVariableInString("targetAzimuth", &V->targetAzimuth, RKValueTypeFloat),
        //     RKVariableInString("markerAzimuth", &V->markerAzimuth, RKValueTypeFloat)
        // );
        switch (scan->mode) {
            case RKScanModeRHI:
            case RKScanModeSpeedDown:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                daz = RKMinDiff(scan->azimuthStart, pos->azimuthDegrees);
                udel = fabs(del);
                udaz = fabs(daz);
                if (udel < RKPedestalPositionTolerance && udaz < RKPedestalPositionTolerance) {
                    action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = scan->elevationSlew;
                    action->mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = 0.0f;
                    if (verbose) {
                        RKLog("%s Info. Ready for sweep %d - AZ %.2f @ crossover EL %.2f\033[m\n", engine->name,
                            V->i, scan->azimuthStart, scan->elevationEnd);
                    }
                    V->progress |= RKScanProgressMiddle;
                    V->progress ^= RKScanProgressSetup;
                    V->tic = 0;
                } else {
                    if (udaz >= RKPedestalPositionTolerance) {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                        action->param[a] = RKSteerEngineGetRate(daz, RKPedestalAxisAzimuth);
                        V->tic = 0;
                        a++;
                    } else if (pos->azimuthVelocityDegreesPerSecond > 0.0f) {
                        action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                        action->mode[a] = 0.0f;
                        V->tic = 0;
                        a++;
                    }
                    if (udel >= RKPedestalPositionTolerance) {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->param[a] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                        V->tic = 0;
                    } else if (pos->elevationVelocityDegreesPerSecond > 0.0f) {
                        action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                        action->mode[a] = 0.0f;
                        V->tic = 0;
                    }
                }
                break;
            case RKScanModePPI:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                udel = fabs(del);
                if (udel < RKPedestalPositionTolerance) {
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = 0.0f;
                    action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = scan->azimuthSlew;
                    if (verbose) {
                        RKLog("%s Info. Ready for sweep %d - EL %.2f @ crossover AZ %.2f\033[m\n", engine->name,
                            V->i, scan->elevationStart, scan->azimuthMark);
                    }
                    V->progress ^= RKScanProgressSetup;
                    V->tic = 0;
                } else {
                    action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                    V->tic = 0;
                }
                break;
            case RKScanModeSector:
                del = RKMinDiff(scan->elevationStart, pos->elevationDegrees);
                daz = RKMinDiff(scan->azimuthStart, pos->azimuthDegrees);
                udel = fabs(del);
                udaz = fabs(daz);
                if (udel < RKPedestalPositionTolerance && udaz < RKPedestalPositionTolerance) {
                    action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[0] = scan->azimuthSlew;
                    action->mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->param[1] = 0.0f;
                    if (verbose) {
                        RKLog("%s Info. Ready for sweep %d - EL %.2f @ crossover AZ %.2f\033[m\n", engine->name,
                            V->i, scan->elevationStart, scan->azimuthEnd);
                    }
                    V->progress |= RKScanProgressMiddle;
                    V->progress ^= RKScanProgressSetup;
                    V->tic = 0;
                } else {
                    if (udaz >= RKPedestalPositionTolerance) {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                        action->param[a] = RKSteerEngineGetRate(daz, RKPedestalAxisAzimuth);
                        V->tic = 0;
                        a++;
                    } else if (pos->azimuthVelocityDegreesPerSecond > 0.0f) {
                        action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                        action->mode[a] = 0.0f;
                        V->tic = 0;
                        a++;
                    }
                    if (udel >= RKPedestalPositionTolerance) {
                        action->mode[a] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->param[a] = RKSteerEngineGetRate(del, RKPedestalAxisElevation);
                        V->tic = 0;
                    } else if (pos->elevationVelocityDegreesPerSecond > 0.0f) {
                        action->mode[a] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                        action->mode[a] = 0.0f;
                        V->tic = 0;
                    }
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
                if (pos->elevationVelocityDegreesPerSecond == 0.0f) {
                    break;
                }
                cross = RKAngularCrossOver(pos->elevationDegrees, V->elevationPrevious, scan->elevationEnd);
                if (cross) {
                    V->progress |= RKScanProgressEnd;
                    V->progress ^= RKScanProgressMiddle;
                    if (verbose) {
                        RKLog("%s Target cross detected @ EL %.2f [%.2f -> %.2f]\n", engine->name,
                            scan->elevationEnd, V->elevationPrevious, pos->elevationDegrees);
                    }
                }
                break;
            case RKScanModePPI:
                // If the action was just sent, pedestal has not yet accelerated
                if (pos->azimuthVelocityDegreesPerSecond == 0.0f) {
                    break;
                }
                // Check for a cross-over trigger
                cross = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, scan->azimuthEnd);
                if (cross) {
                    V->progress |= RKScanProgressEnd;
                    if (verbose) {
                        RKLog("%s Target cross detected @ AZ %.2f [%.2f -> %.2f]\n", engine->name,
                            scan->azimuthEnd, V->azimuthPrevious, pos->azimuthDegrees);
                    }
                }
                cross = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, scan->azimuthMark);
                if (cross) {
                    V->progress |= RKScanProgressMarker;
                    if (verbose) {
                        RKLog("%s Marker cross detected @ AZ %.2f [%.2f -> %.2f]\n", engine->name,
                            scan->azimuthMark, V->azimuthPrevious, pos->azimuthDegrees);
                    }
                }
                break;
            case RKScanModeSector:
                // If the action was just sent, pedestal has not yet accelerated
                if (pos->azimuthVelocityDegreesPerSecond == 0.0f) {
                    break;
                }
                cross = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, scan->azimuthEnd);
                if (cross) {
                    V->progress |= RKScanProgressEnd | RKScanProgressMarker;
                    if (verbose) {
                        RKLog("%s Target cross detected @ AZ %.2f [%.2f -> %.2f]\n", engine->name,
                            scan->azimuthEnd, V->azimuthPrevious, pos->azimuthDegrees);
                    }
                }
                break;
            default:
                break;
        }
    }

    RKSteerEngineUpdatePositionFlags(engine, pos);

    RKSteerEngineUpdateStatusString(engine);

    if (V->progress & RKScanProgressMarker) {
        V->progress ^= RKScanProgressMarker;
    }

    if (V->progress & RKScanProgressEnd) {
        V->progress ^= RKScanProgressEnd;
        // Sweep ended, go to the next sweep
        V->i = RKNextModuloS(V->i, V->sweepCount);
        if (V->i == 0) {
            if (V->option & RKScanOptionRepeat) {
                RKSteerEngineNextHitter(engine);
                RKLog("%s VCP repeats.\n", engine->name);
            } else {
                RKLog("%s VCP stops.\n", engine->name);
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

int RKSteerEngineExecuteString(RKSteerEngine *engine, const char *command, char *response) {
    char args[4][256] = {"", "", "", ""};
    const int n = sscanf(command, "%*s %256s %256s %256s %256s", args[0], args[1], args[2], args[3]);

    bool immediatelyDo = false;
    if (!engine->vcpHandle.active) {
        immediatelyDo = true;
    }

    bool onlyOnce = false;

    float azimuthStart, azimuthEnd, azimuthMark, elevationStart, elevationEnd, rate;

    if (engine->verbose > 1 && (*command == 'r' || *command == 'p')) {
        RKLog("%s command = '%s' -> ['%s', '%s', '%s', '%s']\n", engine->name, command, args[0], args[1], args[2], args[3]);
    }

    if (response == NULL) {
        RKLog("%s response cannot be NULL\n", engine->name);
    }
    // char *response = feedback == NULL ? (char *)engine->response : feedback;

    bool everythingOkay = true;

    if (!strncmp("pp", command, 2) || !strncmp("ipp", command, 3) || !strncmp("opp", command, 3)) {

        // pp 2,4,6,8,10 45 18 - PPI at elevations [2, 4, 6, 8, 10] degs, azimuth 45, speed 18 deg/s

        if (n < 2) {
            sprintf(response, "NAK. Ill-defined PPI array.   n = %d" RKEOL, n);
            return RKResultIncompleteScanDescription;
        }

        if (!strncmp("pp", command, 2)) {
            RKSteerEngineClearHole(engine);
        } else if (!strncmp("ipp", command, 3)) {
            RKSteerEngineClearSweeps(engine);
            immediatelyDo = true;
        } else if (!strncmp("opp", command, 3)) {
            RKSteerEngineClearDeck(engine);
            onlyOnce = true;
        }

        char *elevations = args[0];
        char *azimuth = args[1];
        const char comma[] = ",";

        if (n == 1) {
            azimuthStart = 0.0f;
            azimuthEnd = 0.0f;
        }
        if (n > 1) {
            azimuthStart = atof(azimuth);
        } else {
            azimuthEnd = 0.0f;
        }
        azimuthMark = azimuthEnd;
        if (n > 2) {
            rate = atof(args[2]);
        } else {
            rate = RKDefaultScanSpeed;
        }

        char *token = strtok(elevations, comma);

        int k = 0;
        while (token != NULL && k++ < 50) {
            const int m = sscanf(token, "%f", &elevationStart);
            if (m == 0) {
                sprintf(response, "NAK. Ill-defined PPI component.   m = %d" RKEOL, m);
                everythingOkay = false;
                break;
            }
            elevationEnd = elevationStart;
            azimuthEnd = azimuthStart;

            RKScanPath scan = RKSteerEngineMakeScanPath(RKScanModePPI, elevationStart, elevationEnd, azimuthStart, azimuthEnd, azimuthMark, rate);

            if (onlyOnce) {
                RKSteerEngineAddPinchSweep(engine, scan);
            } else{
                RKSteerEngineAddLineupSweep(engine, scan);
            }
            token = strtok(NULL, comma);
        }

    } else if (!strncmp("rr", command, 2) || !strncmp("irr", command, 3) || !strncmp("orr", command, 3)) {

        // rr 0,20 10,20,30 10 - RHI at elevations [0, 20] degs, azimuths [10, 20, 30], speed 10 deg/s

        if (n < 2) {
            sprintf(response, "NAK. Ill-defined RHI array.   n = %d" RKEOL, n);
            return RKResultIncompleteScanDescription;
        }

        if (!strncmp("rr", command, 2)) {
            RKSteerEngineClearHole(engine);
        } else if (!strncmp("irr", command, 3)) {
            RKSteerEngineClearSweeps(engine);
            immediatelyDo = true;
        } else if (!strncmp("orr", command, 3)) {
            RKSteerEngineClearDeck(engine);
            onlyOnce = true;
        }

        char *elevations = args[0];
        char *azimuths = args[1];
        const char comma[] = ",";

        if (n > 2) {
            rate = atof(args[2]);
        } else {
            rate = RKDefaultScanSpeed;
        }

        int m = sscanf(elevations, "%f,%f", &elevationStart, &elevationEnd);
        if (m < 2) {
            sprintf(response, "NAK. Ill-defined RHI component.   m = %d" RKEOL, m);
            everythingOkay = false;
        }

        char *token = strtok(azimuths, comma);

        int k = 0;
        bool flip = false;
        while (token != NULL && everythingOkay && k++ < 180) {
            const int o = sscanf(token, "%f", &azimuthStart);
            if (o == 0) {
                sprintf(response, "NAK. Ill-defined RHI component.   o = %d" RKEOL, m);
                everythingOkay = false;
                break;
            }
            azimuthEnd = azimuthStart;

            RKScanPath scan = flip ?
                RKSteerEngineMakeScanPath(RKScanModeRHI, elevationEnd, elevationStart, azimuthStart, 0, 0, -rate) :
                RKSteerEngineMakeScanPath(RKScanModeRHI, elevationStart, elevationEnd, azimuthStart, 0, 0, rate);

            if (onlyOnce) {
                RKSteerEngineAddPinchSweep(engine, scan);
            } else{
                RKSteerEngineAddLineupSweep(engine, scan);
            }
            token = strtok(NULL, comma);
            flip = !flip;
        }

    } else if (!strncmp("vol", command, 3) || !strncmp("ivol", command, 3) || !strncmp("ovol", command, 3)) {

        if (n < 2) {
            sprintf(response, "NAK. Ill-defined volume array." RKEOL);
            return RKResultIncompleteScanDescription;
        }

        if (!strncmp("vol", command, 3)) {
            RKSteerEngineClearHole(engine);
        } else if (!strncmp("ivol", command, 4)) {
            RKSteerEngineClearSweeps(engine);
            immediatelyDo = true;
        } else if (!strncmp("ovol", command, 4)) {
            RKSteerEngineClearDeck(engine);
            onlyOnce = true;
        }

        const char slash[] = "/";
        char symbol[4];
        int o;

        int k = 0;
        char *token = strchr(command, ' ');
        do {
            token++;
        } while (*token == ' ' && k++ < 10);
        strncpy(engine->scanString, token, RKMaximumStringLength - 1);

        RKScanMode mode = RKScanModeSpeedDown;

        k = 0;
        token = strtok(engine->scanString, slash);
        while (token != NULL && k++ < 100) {
            // parse in the usual way
            const int m = sscanf(token, "%3s %16s %16s %16s", symbol, args[0], args[1], args[2]);
            if (m < 2) {
                sprintf(response, "NAK. Ill-defined volume component.   m = %d" RKEOL, m);
                everythingOkay = false;
                break;
            }
            if (*symbol == 'p') {
                mode = RKScanModePPI;
            } else if (*symbol == 'r') {
                mode = RKScanModeRHI;
            } else if (*symbol == 's') {
                mode = RKScanModeSector;
            } else if (*symbol == 'b') {
                mode = RKScanModeSpeedDown;
            } else {
                RKLog("%s Error. Scan mode '%s' not undersood.\n", engine->name, symbol);
                everythingOkay = false;
                break;
            }
            o = sscanf(args[0], "%f,%f", &elevationStart, &elevationEnd);
            if (o < 1) {
                sprintf(response, "NAK. Ill-defined volume component.   o = %d" RKEOL, o);
                everythingOkay = false;
                break;
            } else if (o == 1) {
                elevationEnd = elevationStart;
            }
            o = sscanf(args[1], "%f,%f,%f", &azimuthStart, &azimuthEnd, &azimuthMark);
            if (o < 1) {
                sprintf(response, "NAK. Ill-defined volume component.   o = %d" RKEOL, o);
                everythingOkay = false;
                break;
            } else if (o == 2) {
                azimuthMark = azimuthEnd;
            } else if (o == 1) {
                azimuthEnd = azimuthStart;
                azimuthMark = azimuthStart;
            }
            rate = atof(args[2]);

            RKScanPath scan = RKSteerEngineMakeScanPath(mode, elevationStart, elevationEnd, azimuthStart, azimuthEnd, azimuthMark, rate);

            if (onlyOnce) {
                RKSteerEngineAddPinchSweep(engine, scan);
            } else {
                RKSteerEngineAddLineupSweep(engine, scan);
            }

            token = strtok(NULL, slash);
        }

    }

    if (everythingOkay) {
        if (immediatelyDo) {
            RKSteerEngineNextHitter(engine);
        }
        RKSteerEngineScanSummary(engine, response);
        sprintf(response + strlen(response), "ACK. Volume added successfully." RKEOL);
        int k = sprintf(engine->dump, "Scan object created.\n");
        RKIndentCopy(engine->dump + k, response, 31);
        RKStripTail(engine->dump);
        RKLog("%s %s\n", engine->name, engine->dump);
    } else {
        return RKResultFailedToSetVCP;
    }

    return RKResultSuccess;
}

RKScanPath RKSteerEngineMakeScanPath(RKScanMode mode,
                                             const float elevationStart, const float elevationEnd,
                                             const float azimuthStart, const float azimuthEnd, const float azimuthMark,
                                             const float rate) {
    RKScanPath sweep = {
        .mode = mode,
        .azimuthStart   = azimuthStart,
        .azimuthEnd     = azimuthEnd,
        .azimuthMark    = azimuthMark,
        .azimuthSlew    = rate,
        .elevationStart = elevationStart,
        .elevationEnd   = elevationEnd,
        .elevationSlew  = rate
    };
    return sweep;
}

#define RKDigitWidth(v, n)  (int)(floorf(log10f(fabsf(v))) + ((v) < 0) + (n) + 2)

static void makeSweepMessage(RKScanPath *scanPaths, char *string, int count, RKScanHitter linetag) {
    char prefix[8];
    switch (linetag){
        case RKScanAtBat:
            strncpy(prefix, "       ", 7);
            break;
        case RKScanPinch:
            strncpy(prefix, "PINCH  ", 7);
            break;
        case RKScanLine:
            strncpy(prefix, "LINEUP ", 7);
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
    int am = 0;
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
        am = MAX(am, RKDigitWidth(scanPaths[i].azimuthMark, n));
        ae = MAX(ae, RKDigitWidth(scanPaths[i].azimuthEnd, n));
        av = MAX(av, RKDigitWidth(scanPaths[i].azimuthSlew, n));
    }

    // Second pass, actually print it out using the consitent column width
    for (int i = 0; i < count; i++) {
        switch (scanPaths[i].mode) {
            case RKScanModePPI:
                sprintf(format, "%%s%%d : PPI EL %%%d.1f   AZ %%%d.1f   @ %%%d.1f deg/s", es, am, av);
                sprintf(string + strlen(string), format,
                        prefix,
                        i,
                        scanPaths[i].elevationStart,
                        scanPaths[i].azimuthMark,
                        scanPaths[i].azimuthSlew);
                break;
            case RKScanModeRHI:
                sprintf(format, "%%s%%d : RHI AZ %%%d.1f   EZ %%%d.1f-%%%d.1f   @ %%%d.1f deg/s", as, es, ee, ev);
                sprintf(string + strlen(string), format,
                        prefix,
                        i,
                        scanPaths[i].azimuthStart,
                        scanPaths[i].elevationStart,
                        scanPaths[i].elevationEnd,
                        scanPaths[i].elevationSlew);
                break;
            case RKScanModeSector:
                sprintf(format, "%%s%%d : SEC EL %%%d.1f   AZ %%%d.1f-%%%d.1f   @ %%%d.1f deg/s", es, as, ae, av);
                sprintf(string + strlen(string), format,
                        prefix,
                        i,
                        scanPaths[i].elevationStart,
                        scanPaths[i].azimuthStart,
                        scanPaths[i].azimuthEnd,
                        scanPaths[i].azimuthSlew);
                break;
            case RKScanModeSpeedDown:
                sprintf(string + strlen(string), "%s%d : PPI_SLOWDOWN",
                        prefix,
                        i);
            default:
                break;
        }
        sprintf(string + strlen(string), "\n");
    }
}


void RKSteerEngineScanSummary(RKSteerEngine *engine, char *string) {
    RKScanObject *V = &engine->vcpHandle;
    string[0] = '\0';
    makeSweepMessage(V->batterScans, string, V->sweepCount, RKScanAtBat);
    if (memcmp(V->inTheHoleScans, V->onDeckScans, (V->onDeckCount + V->inTheHoleCount) * sizeof(RKScanPath))) {
        makeSweepMessage(V->onDeckScans, string, V->onDeckCount, RKScanPinch);
    }
    makeSweepMessage(V->inTheHoleScans, string, V->inTheHoleCount, RKScanLine);
}

char *RKSteerEngineStatusString(RKSteerEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
