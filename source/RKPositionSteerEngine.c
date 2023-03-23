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

    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

float pedestalGetRate(const float diff_deg, RKPedestalAxis axis) {
    float rate = 0.0f;
    if (axis == RKPedestalAxisAzimuth) {
        if (diff_deg >= 20.0f) {
            rate = 30.0f;
        } else if (diff_deg >= 10.0f) {
            rate = 20.0f;
        } else if (diff_deg >= 5.0f) {
            rate = 10.0f;
        } else if (diff_deg < 5.0f) {
            rate = 3.0f;
        }
    } else if (axis == RKPedestalAxisElevation) {
        if (diff_deg >= 20.0f) {
            rate = 15.0f;
        } else if (diff_deg >= 7.0f) {
            rate = 10.0f;
        } else if (diff_deg < 7.0f) {
            rate = 3.0f;
        }
    }
    return rate;
}

//int pedestalElevationPoint(RKPositionSteerEngine *engine, const float el_point, const float rate_az){
//    float umin_diff_el;
//    float min_diff_el;
//    float rate_el;
//
//    //RKPosition *pos = RKGetLatestPosition(me->radar);
//    uint32_t latestPositionIndex = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
//    RKPosition *pos = &engine->positionBuffer[latestPositionIndex];
//
//    RKPedestalAction action;
//    action.mode[0] = RKPedestalInstructTypeNone;
//    action.mode[1] = RKPedestalInstructTypeNone;
//    action.param[0] = 0.0f;
//    action.param[1] = 0.0f;
//    action.sweepElevation = el_point;
//    action.sweepAzimuth = pos->azimuthDegrees;
//    umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//    int i = 0;
//    while (umin_diff_el > RKPedestalPositionTolerance && i < RKPedestalPointTimeOut) {
//
//        // pos = RKGetLatestPosition(me->radar);
//        uint32_t latestPositionIndex = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
//        RKPosition *pos = &engine->positionBuffer[latestPositionIndex];
//        umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//        if (umin_diff_el > RKPedestalPositionTolerance){
//            action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//            rate_el = pedestalGetRate(umin_diff_el, RKPedestalPointElevation);
//            min_diff_el = RKMinDiff(el_point, pos->elevationDegrees);
//            if (min_diff_el >= 0.0f){
//                action.param[0] = rate_el;
//            }else{
//                action.param[0] = -rate_el;
//            }
//        } else{
//            action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
//            action.param[0] = 0.0f;
//        }
//        action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//        action.param[1] = rate_az;
//        // printf("EL point %.2f | AZ slew %.2f dps\n",pos->elevationDegrees,rate_az);
//        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//        i++;
//        usleep(20000);
//    }
//    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
//    action.param[0] = 0.0f;
//    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//    action.param[1] = rate_az;
//    // pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//
//    printf("%s","Elevation point finish.");
//    if (i < RKPedestalPointTimeOut){
//        return 0;
//    }else{
//        return 1;
//    }
//}

RKPedestalAction pedestalElevationPointNudge(RKPositionSteerEngine *engine, const float el_point, const float rate_az) {
    float umin_diff_el;
    float min_diff_el;
    float rate_el;

    // RKPosition *pos = RKGetLatestPosition(me->radar);
    uint32_t latestPositionIndex = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
    RKPosition *pos = &engine->positionBuffer[latestPositionIndex];

    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = el_point;
    action.sweepAzimuth = pos->azimuthDegrees;
    umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
    if (umin_diff_el > RKPedestalPositionTolerance){
        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
        rate_el = pedestalGetRate(umin_diff_el, RKPedestalAxisElevation);
        min_diff_el = RKMinDiff(el_point, pos->elevationDegrees);
        if (min_diff_el >= 0.0f){
            action.param[0] = rate_el;
        }else{
            action.param[0] = -rate_el;
        }
    } else{
        action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
        action.param[0] = 0.0f;
    }
    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
    action.param[1] = rate_az;
    return action;
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

#pragma mark - Delegate Workers

static void *positionSteerer(void *_in) {
    RKPositionSteerEngine *engine = (RKPositionSteerEngine *)_in;

    int k;

    RKLog("%s Started.   mem = %s B   positionIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->positionIndex);

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    k = 0;    // position index
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
    engine->vcpHandle.option = RKScanOptionRepeat;
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

char *RKPositionSteerEngineStatusString(RKPositionSteerEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
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

void RKPositionSteerEngineArmSweeps(RKPositionSteerEngine *engine, const RKScanRepeat repeat) {
    engine->vcpHandle.progress = RKScanProgressNone;
    if (repeat == RKScanRepeatForever) {
        engine->vcpHandle.option |= RKScanOptionRepeat;
    }
    engine->vcpHandle.i = 0;
    engine->vcpHandle.j = 0;
    engine->vcpHandle.active = true;
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

//
//int pedestalVcpAddPinchSweep(RKPedestalVcpHandle *V, RKPedestalVcpSweepHandle sweep) {
//    if (V->onDeckCount < RKPedestalVcpMaxSweeps - 1) {
//        V->onDeckSweeps[V->onDeckCount++] = sweep;
//    } else {
//        fprintf(stderr, "Cannot add more tilts. Currently %d.\n", V->onDeckCount);
//        return 1;
//    }
//    return 0;
//}

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

RKPedestalAction *RKPositionSteerEngineGetAction(RKPositionSteerEngine *engine) {
    float umin_diff_el;
    float umin_diff_az;
    float umin_diff_vel_el;
    float umin_diff_vel_az;

    //RKPosition *pos = RKGetLatestPosition(me->radar);
    uint32_t latestPositionIndex = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
    RKPosition *pos = &engine->positionBuffer[latestPositionIndex];

    RKPedestalAction *action = &engine->actions[engine->actionIndex];
    memset(action, 0, sizeof(RKPedestalAction));
    engine->actionIndex = RKNextModuloS(engine->actionIndex, RKPedestalActionBufferDepth);

    RKPedestalVcpHandle *V = &engine->vcpHandle;

    if (V->sweepCount == 0 || V->active == false) {
        return action;
    }

    float g;
    float target_diff_az;
    float target_diff_el;
    float marker_diff_az;

    target_diff_az = pos->azimuthDegrees - V->batterScans[V->i].azimuthEnd;
    if (target_diff_az < 0.0f) {
        target_diff_az += 360.0f;
    }

    marker_diff_az = pos->azimuthDegrees - V->batterScans[V->i].azimuthMark;
    if (marker_diff_az < 0.0f) {
        marker_diff_az += 360.0f;
    }

    if (V->progress == RKScanProgressNone) {
        // Clear the sweep start mark
        V->progress &= ~RKScanProgressMarker;
        // Reposition the axes
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSector:
            case RKScanModeNewSector:
            case RKScanModeSpeedDown:
            case RKScanModeRHI:
                break;
            case RKScanModePPI:
            case RKScanModePPIAzimuthStep:
            case RKScanModePPIContinuous:
                umin_diff_el = RKUMinDiff(V->batterScans[V->i].elevationStart, pos->elevationDegrees);
                if (umin_diff_el > RKPedestalPositionTolerance){
                    //pedestalElevationPoint(engine, V->batterScans[V->i].elevationStart, V->batterScans[V->i].azimuthSlew);
                    //action = pedestalElevationPointNudge(engine, V->batterScans[V->i].elevationStart, V->batterScans[V->i].azimuthSlew);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                }else{
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = 0.0f;
                    action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = V->batterScans[V->i].azimuthSlew;
                }
                break;
            default:
                break;
        }
        // Setup the start
        V->progress = RKScanProgressSetup;
        V->tic = 0;

    } else if (V->progress == RKScanProgressSetup) {
        // Compute the position difference from the target
        umin_diff_el = RKUMinDiff(V->batterScans[V->i].elevationStart, pos->elevationDegrees);
        umin_diff_az = RKUMinDiff(V->batterScans[V->i].azimuthStart, pos->azimuthDegrees);
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
                V->progress = RKScanProgressReady;
                break;
            case RKScanModePPI:
            case RKScanModePPIAzimuthStep:
            case RKScanModePPIContinuous:
                // Only wait for the EL axis
                if (umin_diff_el < RKPedestalPositionTolerance &&
                    pos->elevationVelocityDegreesPerSecond > -RKPedestalVelocityTolerance &&
                    pos->elevationVelocityDegreesPerSecond < RKPedestalVelocityTolerance) {
                    V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                    V->sweepElevation = V->batterScans[V->i].elevationStart;
                    V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                    printf("\033[1;32mFirst start for sweep %d - EL %.2f  @ crossover AZ %.2f   umin_diff_az=%.2f\033[0m\n", V->i, V->sweepElevation, V->sweepAzimuth, umin_diff_az);
                    V->progress = RKScanProgressReady;
                }
                break;
            default:
                break;
        }
        if (V->tic > RKPedestalPointTimeOut) {
            printf("Too long to stabilize  tic = %d / %d   umin_diff_el=%.2f  umin_diff_az=%.2f  vel=%.2f  vaz=%.2f\n",
                   V->tic, RKPedestalPointTimeOut, umin_diff_el, umin_diff_az, pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
            // Re-arm the sweep
            V->progress = RKScanProgressNone;
            V->tic = 0;
        }

    } else if (V->progress & RKScanProgressReady) {

        // Get the next sweep, set the goals
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSector:
            case RKScanModePPI:
                V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                V->sweepElevation = V->batterScans[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                // V->sweep_az = V->sweeps[V->i].az_start;
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
                umin_diff_el = RKUMinDiff(V->targetElevation, pos->elevationDegrees);
                if (umin_diff_el < RKPedestalPositionTolerance) {
                    printf("Lock elevation axis.  param[1] = %.2f\n", action->param[1]);
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    V->counterTargetAzimuth = 0;
                } else {
                    //pedestalElevationPoint(engine, V->targetElevation, V->batterScans[V->i].azimuthSlew);
                    //action = pedestalElevationPointNudge(engine, V->targetElevation, V->batterScans[V->i].azimuthSlew);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                }
                break;
            case RKScanModeRHI:
                V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                V->sweepElevation = V->batterScans[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterScans[V->i].elevationEnd;
                printf("Ready for AZ %.2f\n", V->sweepAzimuth);

                umin_diff_el = RKUMinDiff(V->batterScans[V->i].elevationStart, pos->elevationDegrees);
                umin_diff_az = RKUMinDiff(V->batterScans[V->i].azimuthStart, pos->azimuthDegrees);
                if (umin_diff_el > RKPedestalPositionTolerance || umin_diff_az > RKPedestalPositionTolerance){
                    //pedestalPoint(me, V->batterScans[V->i].elevationStart, V->batterScans[V->i].azimuthStart);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                }
                V->targetElevation = V->batterScans[V->i].elevationEnd - V->batterScans[V->i].elevationStart;
                V->targetAzimuth = V->batterScans[V->i].azimuthStart;
                V->counterTargetElevation = 0;
                action->mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                action->param[0] = V->batterScans[V->i].elevationSlew;
                umin_diff_az = RKUMinDiff(pos->azimuthDegrees, V->targetAzimuth);
                if (umin_diff_az < RKPedestalPositionTolerance) {
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
                V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                V->sweepElevation = V->batterScans[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                printf("\033[1;33mReady for sweep %d - EL %.2f  @ crossover AZ %.2f\033[0m\n", V->i, action->sweepElevation, action->sweepAzimuth);
                umin_diff_el = RKUMinDiff(V->sweepElevation, pos->elevationDegrees);
                if (umin_diff_el > RKPedestalPositionTolerance) {
                    //pedestalElevationPoint(me, V->sweepElevation, V->batterScans[V->i].azimuthSlew);
                    //action = pedestalElevationPointNudge(me, V->sweepElevation, V->batterScans[V->i].azimuthSlew);
                    //
                    //  Replace with returning actions... I think we need additional states
                    //
                } else if ((pos->elevationVelocityDegreesPerSecond > RKPedestalVelocityTolerance ||
                            pos->elevationVelocityDegreesPerSecond < -RKPedestalVelocityTolerance)) {
                    action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action->param[0] = 0.0f;
                    action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action->param[1] = V->batterScans[V->i].azimuthSlew;
                }
                break;
            case RKScanModePPIContinuous:
                V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                V->sweepElevation = V->batterScans[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                umin_diff_el = RKUMinDiff(V->sweepElevation, pos->elevationDegrees);
                if (umin_diff_el < RKPedestalPositionTolerance) {
                    if ((pos->elevationVelocityDegreesPerSecond > RKPedestalVelocityTolerance
                            || pos->elevationVelocityDegreesPerSecond < -RKPedestalVelocityTolerance)) {
                        action->mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                        action->param[0] = 0.0f;
                        action->mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                        action->param[1] = V->batterScans[V->i].azimuthSlew;
                    }
                    V->counterTargetAzimuth = 0;
                }
                break;
            case RKScanModeNewSector:
                V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                V->sweepElevation = V->batterScans[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
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
                break;
            case RKScanModeSpeedDown:
                V->sweepAzimuth = 0.0f;
                V->sweepElevation = 0.0f;
                V->sweepMarkerElevation = 0.0f;
                umin_diff_vel_el = RKUMinDiff(0.0f, pos->elevationVelocityDegreesPerSecond);
                umin_diff_vel_az = RKUMinDiff(0.0f, pos->azimuthVelocityDegreesPerSecond);
                int tic = 0;
                while ((umin_diff_vel_el > RKPedestalVelocityTolerance || umin_diff_vel_az > RKPedestalVelocityTolerance)
                    && tic < RKPedestalPointTimeOut) {
                    // pos = RKGetLatestPosition(me->radar);
                    umin_diff_vel_el = RKUMinDiff(0.0f, pos->elevationVelocityDegreesPerSecond);
                    umin_diff_vel_az = RKUMinDiff(0.0f, pos->azimuthVelocityDegreesPerSecond);
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
        // This stage the sweep is just about to start
        V->progress = RKScanProgressMiddle;
        V->tic = 0;

    } else if (V->progress & RKScanProgressMiddle) {

        // Middle of a sweep, check for sweep completeness
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSector:
            case RKScanModePPI:
                V->counterTargetAzimuth += RKMinDiff(pos->azimuthDegrees, V->azimuthPrevious);
                //printf("V->counter_az = %.2f <- %.2f\n", V->counter_az, diff_az);
                if ((V->batterScans[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                    (V->batterScans[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                    action->mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    V->progress = RKScanProgressEnd | RKScanProgressMarker;
                }
                // If the elevation axis is still moving and the previous command has been a while
                if ((V->tic > 100 && (pos->elevationVelocityDegreesPerSecond < -0.05f ||
                    pos->elevationVelocityDegreesPerSecond > 0.05f))) {
                    target_diff_el = RKUMinDiff(pos->elevationDegrees, V->targetElevation);
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
                // if (V->progress & RKScanProgressMarker) {
                //     V->progress ^= RKScanProgressMarker;
                // } else if ((V->batterScans[V->i].elevationSlew > 0.0f && V->counterTargetElevation >= V->targetElevation) ||
                if ((V->batterScans[V->i].elevationSlew > 0.0f && V->counterTargetElevation >= V->targetElevation) ||
                    (V->batterScans[V->i].elevationSlew < 0.0f && V->counterTargetElevation <= V->targetElevation)) {
                    action->mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
                    V->progress = RKScanProgressEnd | RKScanProgressMarker;
                }
                // If the azimuth axis is still moving and the previous command has been a while
                if (V->tic > 100 && (pos->azimuthVelocityDegreesPerSecond < -RKPedestalVelocityTolerance ||
                pos->azimuthVelocityDegreesPerSecond > RKPedestalVelocityTolerance)) {
                    target_diff_az = RKUMinDiff(pos->azimuthDegrees, V->targetAzimuth);
                    if (target_diff_az < RKPedestalPositionTolerance) {
                        int k = action->mode[0] == RKPedestalInstructTypeNone ? 0 : 1;
                        action->mode[k] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
                        printf("Lock azimuth axis again.  vaz = %.2f\n", pos->azimuthVelocityDegreesPerSecond);
                        V->tic = 0;
                    }
                }
                break;
            case RKScanModePPIAzimuthStep:
                // Check for a cross-over trigger
                //
                // For positive rotation:
                // target_diff_az_prev = prev_az - az_start should be just < 360
                // target_diff_az      = curr_az - az_start should be just > 0
                //
                // For negative rotation:
                // target_diff_az_prev = prev_az - az_start should be just > 0
                // target_diff_az      = curr_az - az_start should be just < 360
                //
                // The difference between them is big, approx. -/+360.0f
                g = target_diff_az - V->targetDiffAzimuthPrevious;
                //printf("AZ %.2f   counter %.2f   %.2f - %.2f = %.2f\n", pos->az_deg, V->counter_target_az, target_diff_az, V->target_diff_az_prev, g);
                if (V->counterTargetAzimuth > 180.0f && (g < -350.0f || g > 350.0f)) {
                    if (V->option & RKScanOptionVerbose) {
                        RKLog("Warning. Target cross over for %.2f detected @ %.2f.  %.2f %.2f\n", V->batterScans[V->i].azimuthEnd, pos->azimuthDegrees, V->targetDiffAzimuthPrevious, target_diff_az);
                    }
                    V->progress |= RKScanProgressEnd;
                    V->counterTargetAzimuth = 0;
                }
                g = marker_diff_az - V->markerDiffAzimuthPrevious;
                // if (V->progress & RKScanProgressMarker) {
                //     V->progress ^= RKScanProgressMarker;
                // } else if (V->counterMarkerAzimuth > 180.0f && (g < -350.0f || g > 350.0f)) {
                if (V->counterMarkerAzimuth > 180.0f && (g < -350.0f || g > 350.0f)) {
                    if (V->option & RKScanOptionVerbose) {
                        RKLog("Warning. Marker cross over for %.2f detected @ %.2f.  %.2f %.2f\n", V->batterScans[V->i].azimuthMark, pos->azimuthDegrees, V->markerDiffAzimuthPrevious, marker_diff_az);
                    }
                    V->progress |= RKScanProgressMarker;
                    V->counterMarkerAzimuth = 0;
                    // Sweep marked, go to the next sweep
                    V->j = RKNextModuloS(V->j, V->sweepCount);
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
                // V->counterTargetAzimuth += RKMinDiff(pos->azimuthDegrees, V->azimuthPrevious);
                // //printf("V->counter_az = %.2f <- %.2f\n", V->counter_az, diff_az);
                // if ((V->batterScans[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                //     (V->batterScans[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                //     action.mode[0] = INSTRUCT_MODE_STANDBY | INSTRUCT_AXIS_AZ;
                //     // Look ahead of to get ready for the next elevation
                //     int k = V->i == V->sweep_count - 1 ? 0 : V->i + 1;
                //     target_diff_el = RKUMinDiff(pos->el_deg, V->sweeps[k].el_start);
                //     if (target_diff_el < VCP_POS_TOL) {
                //         action.mode[1] = INSTRUCT_MODE_POINT | INSTRUCT_AXIS_EL;
                //         action.param[1] = V->sweeps[k].el_start;
                //     } else {
                //         action.mode[1] = INSTRUCT_MODE_RESET | INSTRUCT_AXIS_EL;
                //     }
                //     V->progress = VCP_PROGRESS_ENDED | VCP_PROGRESS_MARKER;
                // } else if (V->option & VCP_OPTION_BRAKE_EL_DURING_SWEEP &&
                //     // If the elevation axis is still moving and the previous command has been a while
                //     (V->tic > 100 && (pos->vel_dps < -0.05f || pos->vel_dps > 0.05f))) {
                //     target_diff_el = RKUMinDiff(pos->el_deg, V->target_el);
                //     if (target_diff_el < VCP_POS_TOL) {
                //         //int k = action.mode[0] == INSTRUCT_NONE ? 0 : 1;
                //         //action.mode[k] = INSTRUCT_AXIS_EL | INSTRUCT_MODE_STANDBY;
                //         action.mode[0] = INSTRUCT_AXIS_EL | INSTRUCT_MODE_STANDBY;
                //         printf("Lock elevation axis again.\n");
                //         V->tic = 0;
                //     }
                // }
                break;
            case RKScanModeSpeedDown:
                V->progress = RKScanProgressEnd;
                break;
            default:
                break;
        } // switch (V->sweeps[V->i].mode) ...
    }

    if (V->progress & RKScanProgressEnd) {

        // Ended the sweep, check for completeness
        switch (V->batterScans[V->i].mode) {
            case RKScanModeSpeedDown:
            case RKScanModeSector:
            case RKScanModePPI:
            case RKScanModeRHI:
                // Sweep ended, wait until the pedestal stops, then go to the next sweep
                if (pos->azimuthVelocityDegreesPerSecond > -RKPedestalVelocityTolerance &&
                    pos->azimuthVelocityDegreesPerSecond < RKPedestalVelocityTolerance) {
                    V->i = (V->i == V->sweepCount - 1) ? 0 : V->i + 1;
                    if (V->i == 0) {
                        if (V->option & RKScanOptionRepeat) {
                            pedestalVcpNextHitter(&engine->vcpHandle);
                            printf("VCP repeats.\n");
                        } else {
                            printf("VCP stops.\n");
                            V->active = false;
                            //action.mode[0] = INSTRUCT_AXIS_EL | INSTRUCT_STANDBY;
                            //action.mode[1] = INSTRUCT_AXIS_AZ | INSTRUCT_STANDBY;
                        }
                    }
                    V->progress |= RKScanProgressReady;
                    V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                    V->sweepElevation = V->batterScans[V->i].elevationStart;
                    V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                }
                break;
            case RKScanModePPIAzimuthStep:
                // Sweep ended, go to the next sweep
                V->i = (V->i == V->sweepCount - 1) ? 0 : V->i + 1;
                if (V->i == 0) {
                    if (V->option & RKScanOptionRepeat) {
                        pedestalVcpNextHitter(&engine->vcpHandle);
                        printf("VCP repeats.\n");
                        // V->sweepElevation = V->batterScans[V->i].elevationStart;
                        // V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                        V->progress |= RKScanProgressReady;
                    } else {
                        printf("VCP stops.\n");
                        V->active = false;
                        V->progress &= ~RKScanProgressMiddle;
                        break;
                    }
                } else {
                    // V->sweepElevation = V->batterScans[V->i].elevationStart;
                    // V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                    V->progress |= RKScanProgressReady;
                }
                V->sweepAzimuth = V->batterScans[V->i].azimuthStart;
                V->sweepElevation = V->batterScans[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterScans[V->i].elevationStart;
                // if (V->batterScans[V->i].mode != RKScanModePPIAzimuthStep) {
                //     V->progress |= RKScanOptionNone;
                // }
                break;
            case RKScanModePPIContinuous:
                break;
                break;
            default:
                break;
        }
        //printf("wait for pedestal to slow down\n");
    } // if (V->progress == ...)

    V->targetDiffAzimuthPrevious = target_diff_az;
    V->markerDiffAzimuthPrevious = marker_diff_az;
    V->azimuthPrevious = pos->azimuthDegrees;
    V->elevationPrevious = pos->elevationDegrees;
    V->tic++;

//    action.sweepElevation = V->sweepMarkerElevation;
//    action.sweepAzimuth = V->sweepAzimuth;

    return action;
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

    for (int i=0; i<count; i++) {
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
        printf("================================================\n"
               "VCP Summary:\n"
               "------------\n"
               "%s"
               "================================================\n", string);
}
