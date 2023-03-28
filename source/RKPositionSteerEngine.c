//
//  RKPositionSteerEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/21/23.
//  Copyright (c) 2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPositionSteerEngine.h>

#pragma mark - Internal Functions

static void RKPositionSteerEngineUpdateStatusString(RKPositionSteerEngine *engine) {
    char *string;
    //RKPedestalVcpHandle *vcpHandle = engine->vcpHandle;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

//    RKPosition *position = RKGetLatestPosition(engine->radar);
    uint32_t index = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
    RKPosition *position = &engine->positionBuffer[index];

    if (engine->vcpHandle.active) {
        sprintf(string, "-- [ VCP sweep %d / %d -- elevation = %.2f -- azimuth = %.2f -- prog %% ] --\n",
                engine->vcpIndex,
                engine->vcpSweepCount,
                position->elevationDegrees,
                position->azimuthDegrees);
    } else {
        sprintf(string, "-- [ VCP inactive ] --\n");
    }

    // printf("%s\n", string);

    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

float pedestalGetRate(const float delta, RKPedestalAxis axis) {
    float rate = 0.0f;
    float speed = fabsf(delta);
    if (axis == RKPedestalAxisAzimuth) {
        if (speed >= 20.0f) {
            rate = 30.0f;
        } else if (speed >= 10.0f) {
            rate = 20.0f;
        } else if (speed >= 5.0f) {
            rate = 10.0f;
        } else if (speed < 5.0f) {
            rate = 3.0f;
        }
    } else if (axis == RKPedestalAxisElevation) {
        if (speed >= 20.0f) {
            rate = 15.0f;
        } else if (speed >= 7.0f) {
            rate = 10.0f;
        } else if (speed < 7.0f) {
            rate = 3.0f;
        }
    }
    return delta < 0.0f ? -rate : rate;
}

void pedestalVcpNextHitter(RKPedestalVcpHandle *V) {
    memcpy(V->batterScans, V->onDeckScans, V->onDeckCount * sizeof(RKScanPath));
    memcpy(V->onDeckScans, V->inTheHoleScans, (V->onDeckCount + V->inTheHoleCount) * sizeof(RKScanPath));
    V->sweepCount = V->onDeckCount;
    V->onDeckCount = V->inTheHoleCount;
    V->i = 0;
    V->j = 0;
    V->active = true;
}

void RKPositionSteerEngineStopSweeps(RKPositionSteerEngine *engine) {
    engine->vcpHandle.active = false;
}

void RKPositionSteerEngineClearSweeps(RKPositionSteerEngine *engine) {
    engine->vcpHandle.progress = RKScanProgressNone;
    engine->vcpHandle.sweepCount = 0;
    engine->vcpHandle.onDeckCount = 0;
    engine->vcpHandle.inTheHoleCount = 0;
    engine->vcpHandle.i = 0;
    engine->vcpHandle.j = 0;
    engine->vcpHandle.active = true;
}

void RKPositionSteerEngineClearHole(RKPositionSteerEngine *engine) {
    engine->vcpHandle.inTheHoleCount = 0;
    engine->vcpHandle.onDeckCount = 0;
}

void RKPositionSteerEngineClearDeck(RKPositionSteerEngine *engine) {
    engine->vcpHandle.onDeckCount = 0;
}

void RKPositionSteerEngineNextHitter(RKPositionSteerEngine *engine) {
    RKPedestalVcpHandle *V = &engine->vcpHandle;
    memcpy(V->batterScans, V->onDeckScans, V->onDeckCount * sizeof(RKScanPath));
    memcpy(V->onDeckScans, V->inTheHoleScans, (V->onDeckCount + V->inTheHoleCount) * sizeof(RKScanPath));
    V->sweepCount = V->onDeckCount;
    V->onDeckCount = V->inTheHoleCount;
    V->i = 0;
    V->j = 0;
    V->active = true;
}

void RKPositionSteerEngineArmSweeps(RKPositionSteerEngine *engine, const RKScanRepeat repeat) {
    engine->vcpHandle.progress = RKScanProgressNone;
    if (repeat == RKScanRepeatForever) {
        engine->vcpHandle.option |= RKScanOptionRepeat;
    }
    engine->vcpHandle.i = 0;
    engine->vcpHandle.j = 0;
    engine->vcpHandle.active = true;
}

int RKPositionSteerEngineAddLineupSweep(RKPositionSteerEngine *engine, const RKScanPath scan) {
    RKPedestalVcpHandle *V = &engine->vcpHandle;
    if (V->inTheHoleCount < RKMaximumScanCount - 1) {
        V->inTheHoleScans[V->inTheHoleCount++] = scan;
        V->onDeckScans[V->onDeckCount++] = scan;
    } else {
        RKLog("%s Error. Cannot add more scans.  V->inTheHoleCount = %d\n", engine->name, V->inTheHoleCount);
        return RKResultTooBig;
    }
    return RKResultSuccess;
}

int RKPositionSteerEngineAddPinchSweep(RKPositionSteerEngine *engine, const RKScanPath scan) {
    RKPedestalVcpHandle *V = &engine->vcpHandle;
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

static void *positionSteerer(void *_in) {
    RKPositionSteerEngine *engine = (RKPositionSteerEngine *)_in;

    int k;

    RKLog("%s Started.   mem = %s B   positionIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->positionIndex);

    // Make vcpHandle seems like it has been forever since the last instruction
    engine->vcpHandle.tic = 99;

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    k = 0;
    while (engine->state & RKEngineStateWantActive) {
        engine->tic++;

        RKPositionSteerEngineUpdateStatusString(engine);

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
        usleep(10000);
    }

    return NULL;
}

// Implementations

#pragma mark - Life Cycle

RKPositionSteerEngine *RKPositionSteerEngineInit(void) {
    RKPositionSteerEngine *engine = (RKPositionSteerEngine *)malloc(sizeof(RKPositionSteerEngine));
    memset(engine, 0, sizeof(RKPositionSteerEngine));
    sprintf(engine->name, "%s<PositionSteerer>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPositionSteerEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    sprintf(engine->vcpHandle.name, "VCP");
    engine->vcpHandle.option = RKScanOptionRepeat | RKScanOptionVerbose;
    engine->vcpHandle.active = false;
    engine->memoryUsage = sizeof(RKPositionSteerEngine);
    engine->state = RKEngineStateAllocated;
    return engine;
}

void RKPositionSteerEngineFree(RKPositionSteerEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKPositionSteerEngineSetVerbose(RKPositionSteerEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKPositionSteerEngineSetInputOutputBuffers(RKPositionSteerEngine *engine, const RKRadarDesc *desc,
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

int RKPositionSteerEngineStart(RKPositionSteerEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->threadId, NULL, positionSteerer, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartRingPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKPositionSteerEngineStop(RKPositionSteerEngine *engine) {
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

void RKPositionSteerEngineUpdatePositionFlags(RKPositionSteerEngine *engine, RKPosition *position) {
    RKPedestalVcpHandle *V = &engine->vcpHandle;
    if (V->active) {
        const int i = V->i;
        position->flag |= RKPositionFlagVCPActive;
        // Sweep is active: data collection portion (i.e., exclude transitions)
        if (V->progress & RKScanProgressReady ||
            V->progress & RKScanProgressMiddle ||
            V->batterScans[i].mode == RKScanModePPIAzimuthStep ||
            V->batterScans[i].mode == RKScanModePPIContinuous) {
            // Always active if any of the conditions above are met
            position->flag |= RKPositionFlagScanActive;
        }
        // Point / sweep flag
        switch (V->batterScans[i].mode) {
            case RKScanModePPI:
            case RKScanModeSector:
            case RKScanModeNewSector:
            case RKScanModePPIAzimuthStep:
            case RKScanModePPIContinuous:
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
            switch (V->batterScans[i].mode) {
                case RKScanModePPI:
                case RKScanModeSector:
                case RKScanModeNewSector:
                case RKScanModePPIAzimuthStep:
                case RKScanModePPIContinuous:
                    position->flag |= RKPositionFlagAzimuthComplete;
                    break;
                case RKScanModeRHI:
                    position->flag |= RKPositionFlagElevationComplete;
                    break;
                default:
                    break;
            }
        }
        position->sweepElevationDegrees = V->sweepElevation;
        position->sweepAzimuthDegrees = V->sweepAzimuth;
    } else {
        engine->vcpIndex = 0;
        engine->vcpSweepCount = 0;
    }
}

RKScanAction *RKPositionSteerEngineGetAction(RKPositionSteerEngine *engine, RKPosition *pos) {
    RKScanAction *action = &engine->actions[engine->actionIndex];
    memset(action, 0, sizeof(RKScanAction));
    engine->actionIndex = RKNextModuloS(engine->actionIndex, RKPedestalActionBufferDepth);

    RKPedestalVcpHandle *V = &engine->vcpHandle;

    if (V->sweepCount == 0 || V->active == false) {
        return action;
    }

    float g;
    const float target_diff_el = RKUMinDiff(pos->elevationDegrees, V->targetElevation);

    // const float del = RKMinDiff(V->batterScans[V->i].elevationStart, pos->elevationDegrees);
    // const float daz = RKMinDiff(V->batterScans[V->i].azimuthEnd, pos->azimuthDegrees);
    const float del = RKMinDiff(V->targetElevation, pos->elevationDegrees);
    const float daz = RKMinDiff(V->targetAzimuth, pos->azimuthDegrees);
    const float udel = fabs(del);
    const float udaz = fabs(daz);

    // if (V->progress == RKScanProgressNone) {
    //     V->progress = RKScanProgressSetup;
    // } else if (V->progress == RKScanProgressSetup) {
    //     V->progress = RKScanProgressReady;
    // } else if (V->progress & RKScanProgressReady) {
    //     V->progress = RKScanProgressMiddle;
    // } else if (V->progress & RKScanProgressMiddle) {
    //     V->progress |= RKScanProgressMarker;
    // }

    if (V->progress == RKScanProgressNone) {
        // Clear the sweep start mark
        V->progress &= ~RKScanProgressMarker;
        // Setup the start
        V->progress = RKScanProgressSetup;
    } else if (V->progress == RKScanProgressSetup) {
        // Declare ready when certain conditions are met
        switch (V->batterScans[V->i].mode) {
            case RKScanModeNewSector:
            case RKScanModeSector:
            case RKScanModeRHI:
            case RKScanModeSpeedDown:
                switch (V->batterScans[V->i].mode) {
                    case RKScanModeSector:
                        V->sweepAzimuth = 0.0f;
                        V->sweepElevation = V->batterScans[V->i].elevationStart;
                        V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                        printf("Ready for EL %.2f\n", V->sweepElevation);
                        break;
                    case RKScanModeRHI:
                        V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                        V->sweepElevation = V->batterScans[V->i].elevationStart;
                        V->sweepMarkerElevation = V->batterScans[V->i].elevationEnd;
                        printf("Ready for AZ %.2f\n", V->sweepAzimuth);
                        break;
                    default:
                        V->sweepAzimuth = 0.0f;
                        V->sweepElevation = 0.0f;
                        V->sweepMarkerElevation = 0.0f;
                        break;
                }
                V->targetElevation = V->batterScans[V->i].elevationStart;
                V->targetAzimuth = V->batterScans[V->i].azimuthStart;
                V->markerAzimuth = V->batterScans[V->i].azimuthMark;
                V->progress = RKScanProgressReady;
                break;
            case RKScanModePPI:
            case RKScanModePPIAzimuthStep:
            case RKScanModePPIContinuous:
                // Only wait for the EL axis
                if (udel < RKPedestalPositionTolerance && fabs(pos->elevationVelocityDegreesPerSecond) < RKPedestalVelocityTolerance) {
                    V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                    V->sweepElevation = V->batterScans[V->i].elevationStart;
                    V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                    printf("\033[1;32mFirst start for sweep %d - EL %.2f  @ crossover AZ %.2f   udaz=%.2f\033[0m\n", V->i, V->sweepElevation, V->sweepAzimuth, udaz);
                    V->progress = RKScanProgressReady;
                } else if (udel >= RKPedestalPositionTolerance) {
                    if (V->tic > RKPedestalActionPeriod) {
                        action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->param[0] = pedestalGetRate(del, RKPedestalAxisElevation);
                        V->tic = 0;
                    }
                } else {
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = 0.0f;
                    action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = V->batterScans[V->i].azimuthSlew;
                    V->tic = 0;
                }
                // if (V->tic > RKPedestalPointTimeOut) {
                //     printf("Too long to stabilize  tic = %d / %d   udel=%.2f  udaz=%.2f  vel=%.2f  vaz=%.2f\n",
                //         V->tic, RKPedestalPointTimeOut, udel, udaz, pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
                //     // Re-arm the sweep
                //     V->progress = RKScanProgressNone;
                //     V->tic = 0;
                // }
                V->targetAzimuth = V->batterScans[V->i].azimuthEnd;
                V->markerAzimuth = V->batterScans[V->i].azimuthMark;
                V->targetElevation = V->batterScans[V->i].elevationStart;
                break;
            default:
                break;
        }

    } else if (V->progress & RKScanProgressReady) {

        // Get the next sweep, set the goals
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSector:
            case RKScanModePPI:
                V->targetAzimuth = V->batterScans[V->i].azimuthEnd - V->batterScans[V->i].azimuthStart;
                if (V->targetAzimuth <= 0.0f && V->batterScans[V->i].azimuthSlew > 0.0f) {
                    V->targetAzimuth += 361.0f;
                } else if (V->targetAzimuth >= 0.0f && V->batterScans[V->i].azimuthSlew < 0.0f) {
                    V->targetAzimuth -= 361.0f;
                }
                //printf("target_az = %.2f\n", V->target_az);
                V->counterTargetAzimuth = 0;
                V->targetElevation = V->batterScans[V->i].elevationStart;
                action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                action->param[1] = V->batterScans[V->i].azimuthSlew;
                //float delta = RKUMinDiff(V->targetElevation, pos->elevationDegrees);
                if (udel < RKPedestalPositionTolerance) {
                    printf("Lock elevation axis.  param[1] = %.2f\n", action->param[1]);
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    V->counterTargetAzimuth = 0;
                    V->progress = RKScanProgressMiddle;
                    V->tic = 0;
                } else {
                    //pedestalElevationPoint(engine, V->targetElevation, V->batterScans[V->i].azimuthSlew);
                    //action = pedestalElevationPointNudge(engine, V->targetElevation, V->batterScans[V->i].azimuthSlew);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                    if (V->tic > RKPedestalActionPeriod) {
                        action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->param[0] = pedestalGetRate(del, RKPedestalAxisElevation);
                        V->tic = 0;
                    }
                }
                break;
            case RKScanModeRHI:
                if (udel > RKPedestalPositionTolerance || udaz > RKPedestalPositionTolerance){
                    //pedestalPoint(me, V->batterScans[V->i].elevationStart, V->batterScans[V->i].azimuthStart);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                    // if (V->tic > RKPedestalActionPeriod) {
                    //     action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                    //     action->mode[0] = pedestalGetRate(del, RKPedestalAxisElevation);
                    //     action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    //     action->mode[1] = pedestalGetRate(daz, RKPedestalAxisAzimuth);
                    //     V->tic = 0;
                    // }
                } else {
                    printf("Ready for RHI @ AZ %.2f\n", V->sweepAzimuth);
                    // V->progress = RKScanProgressMiddle;
                    // V->tic = 0;
                }
                V->targetElevation = V->batterScans[V->i].elevationEnd - V->batterScans[V->i].elevationStart;
                V->targetAzimuth = V->batterScans[V->i].azimuthStart;
                V->counterTargetElevation = 0;
                action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                action->param[0] = V->batterScans[V->i].elevationSlew;
                //udaz = RKUMinDiff(pos->azimuthDegrees, V->targetAzimuth);
                if (udaz < RKPedestalPositionTolerance) {
                    action->mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    V->counterTargetAzimuth = 0;
                } else {
                    //pedestalAzimuthPoint(me, V->targetAzimuth, V->batterScans[V->i].elevationSlew);
                    //action = pedestalAzimuthPointNudge(me, V->targetAzimuth, V->batterScans[V->i].elevationSlew);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                }
                break;
            case RKScanModePPIAzimuthStep:
                V->targetAzimuth = V->batterScans[V->i].azimuthStart;
                V->markerAzimuth = V->batterScans[V->i].azimuthMark;

                if (udel > RKPedestalPositionTolerance) {
                    if (V->tic > RKPedestalActionPeriod) {
                        action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                        action->param[0] = pedestalGetRate(del, RKPedestalAxisElevation);
                        V->tic = 0;
                    }
                } else if (fabs(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance) {
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = 0.0f;
                    action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = V->batterScans[V->i].azimuthSlew;
                    V->progress = RKScanProgressMiddle;
                    V->tic = 0;
                } else {
                    printf("\033[1;33mReady for sweep %d - EL %.2f @ crossover AZ %.2f\033[0m\n",
                        V->i, V->sweepElevation, V->sweepAzimuth);
                    V->progress = RKScanProgressMiddle;
                    V->tic = 0;
                }
                break;
            case RKScanModePPIContinuous:
                if (udel < RKPedestalPositionTolerance) {
                    if ((pos->elevationVelocityDegreesPerSecond > RKPedestalVelocityTolerance ||
                         pos->elevationVelocityDegreesPerSecond < -RKPedestalVelocityTolerance)) {
                        action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                        action->param[0] = 0.0f;
                        action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                        action->param[1] = V->batterScans[V->i].azimuthSlew;
                        V->progress = RKScanProgressEnd;
                        V->tic = 0;
                    }
                    V->counterTargetAzimuth = 0;
                }
                break;
            case RKScanModeNewSector:
                V->targetAzimuth = V->batterScans[V->i].azimuthEnd - pos->azimuthDegrees;
                if (V->batterScans[V->i].azimuthSlew > 0.0 && V->targetAzimuth < 0.0) {
                    V->targetAzimuth += 360.0;
                } else if (V->batterScans[V->i].azimuthSlew < 0.0 && V->targetAzimuth > 0.0) {
                    V->targetAzimuth -= 360.0;
                }
                printf("VCP_PROGRESS_READY: target_az_count = %.2f  (tgt:%.2f - cur:%.2f)\n", V->targetAzimuth, V->batterScans[V->i].azimuthEnd, pos->azimuthDegrees);
                action->mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeSlew;
                action->param[1] = V->batterScans[V->i].azimuthSlew;

                //printf("target_az = %.2f\n", V->target_az);
                V->counterTargetAzimuth = 0;
                V->targetElevation = V->batterScans[V->i].elevationStart;

                V->progress = RKScanProgressMiddle;
                V->tic = 0;
                break;
            case RKScanModeSpeedDown:
                V->sweepAzimuth = 0.0f;
                V->sweepElevation = 0.0f;
                V->sweepMarkerElevation = 0.0f;
                //umin_diff_vel_el = RKUMinDiff(0.0f, pos->elevationVelocityDegreesPerSecond);
                //umin_diff_vel_az = RKUMinDiff(0.0f, pos->azimuthVelocityDegreesPerSecond);
                int tic = 0;
                //while ((umin_diff_vel_el > RKPedestalVelocityTolerance || umin_diff_vel_az > RKPedestalVelocityTolerance)
                while ((fabs(pos->elevationVelocityDegreesPerSecond) > RKPedestalVelocityTolerance ||
                        fabs(pos->azimuthVelocityDegreesPerSecond) > RKPedestalVelocityTolerance)
                        && tic < RKPedestalPointTimeOut) {
                    // pos = RKGetLatestPosition(me->radar);
                    //umin_diff_vel_el = RKUMinDiff(0.0f, pos->elevationVelocityDegreesPerSecond);
                    //umin_diff_vel_az = RKUMinDiff(0.0f, pos->azimuthVelocityDegreesPerSecond);
                    action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = pos->elevationVelocityDegreesPerSecond * 0.8;
                    action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = pos->azimuthVelocityDegreesPerSecond * 0.8;
                    printf("EL slew %.2f | AZ slew %.2f dps\n",
                        pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
                    //pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
                    //
                    //  Replace with returning actions... need additional states?
                    //
                    tic++;
                    usleep(20000);
                }
                action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                action->param[0] = 0;
                action->mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                action->param[1] = 0;
                printf("%s","Pedestal break finish.");
                V->progress = RKScanProgressEnd;
                break;
            default:
                break;
        }

    }

    if (V->progress & RKScanProgressMiddle) {

        // Middle of a sweep, check for sweep completeness
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSector:
            case RKScanModePPI:
                V->counterTargetAzimuth += RKMinDiff(pos->azimuthDegrees, V->azimuthPrevious);
                //printf("V->counter_az = %.2f <- %.2f\n", V->counter_az, daz);
                if ((V->batterScans[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                    (V->batterScans[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                    action->mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    V->progress = RKScanProgressEnd | RKScanProgressMarker;
                }
                // If the elevation axis is still moving and the previous command has been a while
                if ((V->tic > 100 && (pos->elevationVelocityDegreesPerSecond < -0.05f ||
                    pos->elevationVelocityDegreesPerSecond > 0.05f))) {
                    if (target_diff_el < RKPedestalPositionTolerance) {
                        int k = action->mode[0] == RKPedestalInstructTypeNone ? 0 : 1;
                        action->mode[k] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
                        printf("Lock elevation axis again.  k = %d\n", k);
                        V->tic = 0;
                    }
                }
                break;
            case RKScanModeRHI:
                V->counterTargetElevation += RKMinDiff(pos->elevationDegrees, V->elevationPrevious);
                if ((V->batterScans[V->i].elevationSlew > 0.0f && V->counterTargetElevation >= V->targetElevation) ||
                    (V->batterScans[V->i].elevationSlew < 0.0f && V->counterTargetElevation <= V->targetElevation)) {
                    action->mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
                    V->progress = RKScanProgressEnd | RKScanProgressMarker;
                }
                // If the azimuth axis is still moving and the previous command has been a while
                if (V->tic > 100 && (pos->azimuthVelocityDegreesPerSecond < -RKPedestalVelocityTolerance ||
                                     pos->azimuthVelocityDegreesPerSecond > RKPedestalVelocityTolerance)) {
                    if (udaz < RKPedestalPositionTolerance) {
                        int k = action->mode[0] == RKPedestalInstructTypeNone ? 0 : 1;
                        action->mode[k] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
                        printf("Lock azimuth axis again.  vaz = %.2f\n", pos->azimuthVelocityDegreesPerSecond);
                        V->tic = 0;
                    }
                }
                break;
            case RKScanModePPIAzimuthStep:
                // Check for a cross-over trigger
                bool targetCrossover = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, V->targetAzimuth);
                if (targetCrossover) {
                    if (V->option & RKScanOptionVerbose) {
                        RKLog("%s Target cross detected @ AZ %.2f [%.2f -> %.2f] %s\n", engine->name,
                            V->targetAzimuth, pos->azimuthDegrees, V->azimuthPrevious,
                            RKVariableInString("V->markerAzimuth", &V->markerAzimuth, RKValueTypeFloat));
                    }
                    V->progress |= RKScanProgressEnd;
                    V->counterTargetAzimuth = 0;
                }
                bool markerCrossover = RKAngularCrossOver(pos->azimuthDegrees, V->azimuthPrevious, V->markerAzimuth);
                if (markerCrossover) {
                    if (V->option & RKScanOptionVerbose) {
                        RKLog("%s Marker cross detected @ AZ %.2f [%.2f -> %.2f]\n", engine->name,
                            V->markerAzimuth, pos->azimuthDegrees, V->azimuthPrevious);
                    }
                    V->progress |= RKScanProgressMarker;
                }
                g = RKUMinDiff(pos->azimuthDegrees, V->azimuthPrevious);
                V->counterTargetAzimuth += g;
                V->counterMarkerAzimuth += g;
                break;
            case RKScanModePPIContinuous:
                V->counterTargetAzimuth += RKMinDiff(pos->azimuthDegrees, V->azimuthPrevious);
                if ((V->batterScans[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                    (V->batterScans[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                    V->progress |= RKScanProgressEnd;
                    V->counterTargetAzimuth = 0;
                }
                break;
            case RKScanModeNewSector:
                break;
            case RKScanModeSpeedDown:
                V->progress = RKScanProgressEnd;
                break;
            default:
                break;
        } // switch (V->sweeps[V->i].mode) ...
    }

    RKPositionSteerEngineUpdatePositionFlags(engine, pos);

    RKLog("%s %s%s%s%s  T%3d  EL %.2f / %.2f @ %5.1f °/s (%.2f, %.1f)   AZ%7.2f @ %5.2f °/s (%.2f, %.1f)  M%d '%s%s%s'\n",
        engine->name,
        V->progress & RKScanProgressMarker ? RKMonokaiGreen "M" RKNoColor : ".",
        V->progress & RKScanProgressSetup  ? RKMonokaiOrange "S" RKNoColor : ".",
        V->progress & RKScanProgressMiddle ? "m" : ".",
        V->progress & RKScanProgressEnd    ? RKMonokaiGreen "E" RKNoColor : ".",
        V->tic,
        pos->sweepElevationDegrees,
        pos->elevationDegrees, pos->elevationVelocityDegreesPerSecond, del, V->counterTargetElevation,
        pos->azimuthDegrees, pos->azimuthVelocityDegreesPerSecond, daz, V->counterTargetAzimuth,
        V->batterScans[V->i].mode,
        RKInstructIsNone(action->mode[0]) ? "" : RKMonokaiGreen,
        RKPedestalActionString(action),
        RKInstructIsNone(action->mode[0]) ? "" : RKNoColor);

    if (V->progress & RKScanProgressMarker) {
        V->progress ^= RKScanProgressMarker;
    }

    if (V->progress & RKScanProgressEnd) {
        V->progress ^= RKScanProgressEnd;
        // Ended the sweep, check for completeness
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSpeedDown:
            case RKScanModeSector:
            case RKScanModePPI:
            case RKScanModeRHI:
                // Sweep ended, wait until the pedestal stops, then go to the next sweep
                if (fabs(pos->azimuthVelocityDegreesPerSecond) < RKPedestalVelocityTolerance) {
                    V->i = RKNextModuloS(V->i, V->sweepCount);
                    // if (V->i == 0) {
                    //     if (V->option & RKScanOptionRepeat) {
                    //         pedestalVcpNextHitter(&engine->vcpHandle);
                    //         RKLog("%s VCP repeats.\n", engine->name);
                    //     } else {
                    //         RKLog("%s VCP stops.\n", engine->name);
                    //         V->active = false;
                    //     }
                    // }
                    // V->progress |= RKScanProgressReady;
                    // V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                    // V->sweepElevation = V->batterScans[V->i].elevationStart;
                    // V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                }
                break;
            case RKScanModePPIAzimuthStep:
                // Sweep ended, go to the next sweep
                V->i = RKNextModuloS(V->i, V->sweepCount);
                V->targetAzimuth = V->batterScans[V->i].azimuthStart;
                V->markerAzimuth = V->batterScans[V->i].azimuthMark;
                V->targetElevation = V->batterScans[V->i].elevationStart;

                RKLog("%s %s   %s\n", engine->name,
                    RKVariableInString("targetAzimuth", &V->targetAzimuth, RKValueTypeFloat),
                    RKVariableInString("markerAzimuth", &V->markerAzimuth, RKValueTypeFloat)
                );
                break;
            case RKScanModePPIContinuous:
                break;
            default:
                break;
        }
        if (V->i == 0) {
            if (V->option & RKScanOptionRepeat) {
                pedestalVcpNextHitter(&engine->vcpHandle);
                RKLog("%s VCP repeats.\n", engine->name);
            } else {
                RKLog("%s VCP stops.\n", engine->name);
                V->active = false;
            }
        }
        V->progress |= RKScanProgressReady;
        V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
        V->sweepElevation = V->batterScans[V->i].elevationStart;
        V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
        //printf("wait for pedestal to slow down\n");
    } // if (V->progress == ...)

    V->azimuthPrevious = pos->azimuthDegrees;
    V->elevationPrevious = pos->elevationDegrees;
    V->tic++;

//    action.sweepElevation = V->sweepMarkerElevation;
//    action.sweepAzimuth = V->sweepAzimuth;

    return action;
}

int RKPositionSteerEngineExecuteString(RKPositionSteerEngine *engine, const char *command, char *response) {
    // pp 2,4,6,8,10 45 18 - PPI at elevations [2, 4, 6, 8, 10] degs, azimuth 45, speed 18 deg/s
    char args[4][256] = {"", "", "", ""};
    const int n = sscanf(command, "%*s %256s %256s %256s %256s", args[0], args[1], args[2], args[3]);

    bool immediatelyDo = false;
    bool onlyOnce = false;

    float azimuthStart, azimuthEnd, azimuthMark, elevationStart, elevationEnd, rate;

    RKLog("%s command = '%s' -> ['%s', '%s', '%s', '%s']\n", engine->name, command, args[0], args[1], args[2], args[3]);

    if (response == NULL) {
        response = (char *)engine->response;
    }

    if ((!strncmp("pp", command, 2) || !strncmp("ipp", command, 3) || !strncmp("opp", command, 3))) {
        if (n < 1) {
            sprintf(response, "NAK. Ill-defined PPI array, n = %d" RKEOL, n);
            printf("%s", response);
            return RKResultIncompleteScanDescription;
        }

        char *elevations = args[0];
        char *azimuth = args[1];
        const char comma[] = ",";

        if (!engine->vcpHandle.active) {
            immediatelyDo = true;
        }

        if (!strncmp("pp", command, 2)) {
            RKPositionSteerEngineClearHole(engine);
        } else if (!strncmp("ipp", command, 3)) {
            RKPositionSteerEngineClearSweeps(engine);
            immediatelyDo = true;
        } else if (!strncmp("opp", command, 3)) {
            RKPositionSteerEngineClearDeck(engine);
            onlyOnce = true;
        }

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
        bool everythingOkay = true;
        while (token != NULL && k++ < 50) {
            int m = sscanf(token, "%f", &elevationStart);
            if (m == 0) {
                everythingOkay = false;
                break;
            }
            elevationEnd = elevationStart;
            azimuthEnd = azimuthStart;

            RKScanPath scan = RKPositionSteerEngineMakeScanPath(RKScanModePPIAzimuthStep, elevationStart, elevationEnd, azimuthStart, azimuthEnd, azimuthMark, rate);

            if (onlyOnce) {
                RKPositionSteerEngineAddPinchSweep(engine, scan);
            } else{
                RKPositionSteerEngineAddLineupSweep(engine, scan);
            }
            token = strtok(NULL, comma);
        }

        if (everythingOkay) {
            if (immediatelyDo) {
                RKPositionSteerEngineNextHitter(engine);
            }
            RKPositionSteerEngineScanSummary(engine, response);
            sprintf(response + strlen(response), "ACK. Volume added successfully." RKEOL);
            printf("%s", response);
        } else {
            printf("NAK. Some error occurred." RKEOL);
        }
    }

    return RKResultSuccess;
}

RKScanPath RKPositionSteerEngineMakeScanPath(RKScanMode mode,
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

    for (int i = 0; i < count; i++) {
        switch (scanPaths[i].mode) {
            case RKScanModePPI:
                sprintf(string + strlen(string), "%s%d : PPI E%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        scanPaths[i].elevationStart,
                        scanPaths[i].azimuthSlew);
                break;
            case RKScanModeSector:
            case RKScanModeNewSector:
                sprintf(string + strlen(string), "%s%d : SEC E%.2f A%.2f-%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        scanPaths[i].elevationStart,
                        scanPaths[i].azimuthStart,
                        scanPaths[i].azimuthEnd,
                        scanPaths[i].azimuthSlew);
                break;
            case RKScanModeRHI:
                sprintf(string + strlen(string), "%s%d : RHI A%.2f E%.2f-%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        scanPaths[i].azimuthStart,
                        scanPaths[i].elevationStart,
                        scanPaths[i].elevationEnd,
                        scanPaths[i].elevationSlew);
                break;
            case RKScanModePPIAzimuthStep:
            case RKScanModePPIContinuous:
                sprintf(string + strlen(string), "%s%d : PPI_NEW E%.2f A%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        scanPaths[i].elevationStart,
                        scanPaths[i].azimuthMark,
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


void RKPositionSteerEngineScanSummary(RKPositionSteerEngine *engine, char *string) {
    RKPedestalVcpHandle *V = &engine->vcpHandle;
    string[0] = '\0';
    makeSweepMessage(V->batterScans, string, V->sweepCount, RKScanAtBat);
    if (memcmp(V->inTheHoleScans, V->onDeckScans, (V->onDeckCount + V->inTheHoleCount) * sizeof(RKScanPath))) {
        makeSweepMessage(V->onDeckScans, string, V->onDeckCount, RKScanPinch);
    }
    makeSweepMessage(V->inTheHoleScans, string, V->inTheHoleCount, RKScanLine);
}

char *RKPositionSteerEngineStatusString(RKPositionSteerEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
