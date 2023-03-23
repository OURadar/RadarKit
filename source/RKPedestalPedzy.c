//
//  RKPedestalPedzy.c
//  RadarKit
//
//  Created by Boonleng Cheong on 1/4/17.
//
//  Significant contributions from Ming-Duan Tze in 2023
//
//  Copyright © 2017-2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPedestalPedzy.h>

#pragma mark - Internal Functions

static int RKPedestalPedzyRead(RKClient *client) {
    // The shared user resource pointer
    RKPedestalPedzy *me = (RKPedestalPedzy *)client->userResource;

    RKRadar *radar = me->radar;

    RKPedestalVcpHandle *vcpHandle = &radar->positionSteerEngine->vcpHandle;

    RKPositionFlag flag;

    if (client->netDelimiter.type == 'p') {
        // The payload just read by RKClient
        RKPosition *position = (RKPosition *)client->userPayload;
        if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
            RKLog("Position   %010ld   %010ld   %08x EL %.2f  AZ %.2f --> %d\n",
                  position->i,
                  position->tic,
                  position->flag, position->elevationDegrees, position->azimuthDegrees, *radar->positionEngine->positionIndex);
        }
        // Get a vacant slot for position from Radar, copy over the data, then set it ready
        RKPosition *newPosition = RKGetVacantPosition(radar);
        if (newPosition == NULL) {
            RKLog("%s Error. Failed to get a vacant position.\n", client->name);
            return RKResultFailedToGetVacantPosition;
        }
        // Unset the time and flag prior to memcpy
        position->time.tv_sec = 0;
        position->time.tv_usec = 0;
        position->timeDouble = 0.0;
        position->flag &= RKPositionFlagHardwareMask;
        memcpy(newPosition, client->userPayload, sizeof(RKPosition));
        // Correct by radar heading
        newPosition->azimuthDegrees += radar->desc.heading + me->headingOffset;
        if (newPosition->azimuthDegrees < 0.0f) {
            newPosition->azimuthDegrees += 360.0f;
        } else if (newPosition->azimuthDegrees >= 360.0f) {
            newPosition->azimuthDegrees -= 360.0f;
        }
        newPosition->sweepAzimuthDegrees += radar->desc.heading + me->headingOffset;
        if (newPosition->sweepAzimuthDegrees < 0.0f) {
            newPosition->sweepAzimuthDegrees += 360.0f;
        } else if (newPosition->sweepAzimuthDegrees >= 360.0f) {
            newPosition->sweepAzimuthDegrees -= 360.0f;
        }

        //
        // Add other flags based on radar->positionSteerEngine
        //
//        if (vcpHandle->active) {
//            newPosition->flag |= RKPositionFlagVCPActive;
//            // Sweep is active: data collection portion (i.e., exclude transitions)
//            if (vcpHandle->progress & RKScanProgressReady ||
//                vcpHandle->progress & RKScanProgressMiddle ||
//                vcpHandle->batterSweeps[i].mode == RKVcpModePPIAzimuthStep ||
//                vcpHandle->batterSweeps[i].mode == RKVcpModePPIContinuous) {
//                // Always active if any of the conditions above are met
//                newPosition->flag |= RKPositionFlagScanActive;
//            }
//            // Point / sweep flag
//            switch (vcpHandle->batterSweeps[i].mode) {
//                case RKVcpModePPI:
//                case RKVcpModeSector:
//                case RKVcpModeNewSector:
//                case RKVcpModePPIAzimuthStep:
//                case RKVcpModePPIContinuous:
//                    newPosition->flag |= RKPositionFlagAzimuthSweep;
//                    newPosition->flag |= RKPositionFlagElevationPoint;
//                    //pos->flag &= ~POSITION_FLAG_EL_SWEEP;
//                    break;
//                case RKVcpModeRHI:
//                    newPosition->flag |= RKPositionFlagAzimuthPoint;
//                    newPosition->flag |= RKPositionFlagElevationSweep;
//                    //pos->flag &= ~POSITION_FLAG_AZ_SWEEP;
//                    break;
//                default:
//                    newPosition->flag |= RKPositionFlagAzimuthPoint | RKPositionFlagElevationPoint;
//                    break;
//            }
//            // Completion flag
//            if (vcpHandle->progress & RKScanProgressMarker) {
//                vcpHandle->progress ^= RKScanProgressMarker;
//                switch (vcpHandle->batterSweeps[i].mode) {
//                    case RKVcpModePPI:
//                    case RKVcpModeSector:
//                    case RKVcpModeNewSector:
//                    case RKVcpModePPIAzimuthStep:
//                    case RKVcpModePPIContinuous:
//                        newPosition->flag |= RKPositionFlagAzimuthComplete;
//                        break;
//                    case RKVcpModeRHI:
//                        newPosition->flag |= RKPositionFlagElevationComplete;
//                        break;
//                    default:
//                        break;
//                }
//            }
//        } else {
//            radar->positionSteerEngine.vcpIndex = 0;
//            radar->positionSteerEngine.vcpSweepCount = 0;
//        }

//        RKPositionSteerEngineUpdatePositionFlags(radar->positionSteerEngine, newPosition);
//        RKUpdatePositionFlags(radar, newPosition);
        RKSetPositionReady(radar, newPosition);

        // Get the latest action, could be null
        // RKPedestalAction *action = RKPositionSteerEngineGetAction();
        // if (action is not do nothing
        //


    } else {
        // This the command acknowledgement, queue it up to feedback
        char *string = (char *)client->userPayload;
        if (!strncmp(string, "pong", 4)) {
            // Just a beacon response.
        } else {
            strncpy(me->responses[me->responseIndex], client->userPayload, RKMaximumStringLength - 1);
            me->responseIndex = RKNextModuloS(me->responseIndex, RKPedestalPedzyFeedbackDepth);
            if (client->verbose > 1 && me->latestCommand[0] != 'h') {
                RKStripTail(string);
                RKLog("%s %s", client->name, string);
            }
        }
    }
    return RKResultSuccess;
}

static int RKPedestalPedzyGreet(RKClient *client) {
    // The shared user resource pointer
    RKPedestalPedzy *me = (RKPedestalPedzy *)client->userResource;
    RKRadar *radar = me->radar;
    if (client->verbose > 1) {
        RKLog("%s Resetting position clock.\n", me->client->name);
    }
    RKClockReset(radar->positionClock);
    return RKResultSuccess;
}

#pragma mark - Delegate Workers

//static void *pedestalVcpEngine(void *in) {
//    int i, k;
//    RKPedestalPedzy *me = (RKPedestal)in;
//    RKRadar *radar = me->radar;
//    RKClient *client = me->client;
//    RKPedestalVcpHandle *vcpHandle = me->vcpHandle;
//    RKStatusEnum vcpActive;
//
//    gettimeofday(&RKPedestalVcpCurrentTime, NULL);
//    timerclear(&RKPedestalVcpStatusTriggerTime);
//    timerclear(&RKPedestalVcpStatusPeriod);
//    RKPedestalVcpStatusPeriod.tv_sec = RKPedestalVcpStatusPeriodMS / 1000;
//    RKPedestalVcpStatusPeriod.tv_usec = (RKPedestalVcpStatusPeriodMS % 1000) * 1000;
//    timeradd(&RKPedestalVcpCurrentTime, &RKPedestalVcpStatusPeriod, &RKPedestalVcpStatusTriggerTime);
//    RKPedestalAction action;
//
//    k = radar->positionIndex;
//
//    while (me->client->state < RKClientStateDisconnecting) {
//        gettimeofday(&RKPedestalVcpCurrentTime, NULL);
//        if (me->client->state < RKClientStateConnected) {
//            vcpActive = RKStatusEnumInvalid;
//        } else {
//            //
//            //   To be replaced with continuous position data ... -boonleng
//            //
//            RKPosition *position = RKGetLatestPosition(radar);
//            if (position->flag & (RKPositionFlagAzimuthError | RKPositionFlagElevationError)) {
//                vcpActive = RKStatusEnumFault;
//            } else if (position->flag & RKPositionFlagVCPActive) {
//                vcpActive = RKStatusEnumActive;
//            } else {
//                vcpActive = RKStatusEnumStandby;
//            }
//
//            if (vcpHandle->active){
//                // put vcp get action here
//                action = pedestalVcpGetAction(me);
//                // action.sweepElevation = vcpHandle->batterSweeps[vcpHandle->i].elevationStart;
//                // action.sweepAzimuth = vcpHandle->batterSweeps[vcpHandle->i].azimuthStart;
//                vcpHandle->lastAction = action;
//                // Call the control handler based on the suggested action
//                if (action.mode[0] != RKPedestalInstructTypeNone || action.mode[1] != RKPedestalInstructTypeNone) {
//                    // P->control(P->ped, action);
//                    pedestalVcpSendAction(client->sd, me->latestCommand, &action);
//
//                    for (i = 0; i < 2; i++) {
//                        if (action.mode[i] != RKPedestalInstructTypeNone) {
//                            RKLog("action.mode[%d] %s%s%s%s%s%s %.2f\n", i,
//                               InstructIsAzimuth(action.mode[i]) ? "AZ"       : "EL",
//                               InstructIsPoint(action.mode[i])   ? " point"   : "",
//                               InstructIsSlew(action.mode[i])    ? " slew"    : "",
//                               InstructIsStandby(action.mode[i]) ? " standby" : "",
//                               InstructIsEnable(action.mode[i])  ? " enable"  : "",
//                               InstructIsDisable(action.mode[i]) ? " disable" : "",
//                               action.param[i]);
//                        }
//                    }
//                    me->lastActionAge = 0;
//                }
//            }
//            if (timercmp(&RKPedestalVcpCurrentTime, &RKPedestalVcpStatusTriggerTime, >=)) {
//                timeradd(&RKPedestalVcpCurrentTime, &RKPedestalVcpStatusPeriod, &RKPedestalVcpStatusTriggerTime);
//                if (vcpHandle->active) {
//                    printf("                 -- [ VCP sweep %d / %d -- elevation = %.2f -- azimuth = %.2f -- prog %d ] --\n",
//                           vcpHandle->i,
//                           vcpHandle->sweepCount,
//                        position->elevationDegrees,
//                        position->azimuthDegrees,
//                           vcpHandle->progress);
//                } else {
//                    printf("                 -- [ VCP inactive ] --\n");
//                }
//            }
//        }
//        usleep(20000);
//        me->lastActionAge++;
//    }
//    return (void *)NULL;
//}

static void *pedestalHealth(void *in) {
    RKPedestalPedzy *me = (RKPedestal)in;
    RKRadar *radar = me->radar;
    RKStatusEnum azInterlockStatus = RKStatusEnumInvalid;
    RKStatusEnum elInterlockStatus = RKStatusEnumInvalid;
    RKStatusEnum vcpActive;
    char azPosition[16];
    char elPosition[16];
    RKStatusEnum azEnum;
    RKStatusEnum elEnum;
    while (me->client->state < RKClientStateDisconnecting) {
        if (me->client->state < RKClientStateConnected) {
            azInterlockStatus = RKStatusEnumInvalid;
            elInterlockStatus = RKStatusEnumInvalid;
            vcpActive = RKStatusEnumInvalid;
            sprintf(elPosition, "--.-- deg");
            sprintf(azPosition, "--.-- deg");
            azEnum = RKStatusEnumInvalid;
            elEnum = RKStatusEnumInvalid;
        } else {
            RKPosition *position = RKGetLatestPosition(radar);
            RKLog("position = %.2f %.2f\n", position->azimuthDegrees, position->elevationDegrees);
            azInterlockStatus = position->flag & RKPositionFlagAzimuthSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            elInterlockStatus = position->flag & RKPositionFlagElevationSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            if (position->flag & (RKPositionFlagAzimuthError | RKPositionFlagElevationError)) {
                vcpActive = RKStatusEnumFault;
            //} else if (position->flag & RKPositionFlagVCPActive) {
            } else if (radar->positionSteerEngine->vcpHandle.active) {
                vcpActive = RKStatusEnumActive;
            } else {
                vcpActive = RKStatusEnumStandby;
            }
            sprintf(elPosition, "%.2f deg", position->elevationDegrees);
            sprintf(azPosition, "%.2f deg", position->azimuthDegrees);
            azEnum = RKStatusEnumNormal;
            elEnum = RKStatusEnumNormal;
        }
        
        RKHealth *health = RKGetVacantHealth(radar, RKHealthNodePedestal);
        if (health) {
            double rate = RKGetPositionUpdateRate(radar);
            sprintf(health->string,
                    "{\"Pedestal AZ Interlock\":{\"Value\":%s,\"Enum\":%d}, "
                    "\"Pedestal EL Interlock\":{\"Value\":%s,\"Enum\":%d}, "
                    "\"VCP Active\":{\"Value\":%s,\"Enum\":%d}, "
                    "\"Pedestal AZ\":{\"Value\":\"%s\",\"Enum\":%d}, "
                    "\"Pedestal EL\":{\"Value\":\"%s\",\"Enum\":%d}, "
                    "\"Pedestal Update\":\"%.3f Hz\", "
                    "\"PedestalHealthEnd\":0}",
                    azInterlockStatus == RKStatusEnumActive ? "true" : "false", azInterlockStatus,
                    elInterlockStatus == RKStatusEnumActive ? "true" : "false", elInterlockStatus,
                    vcpActive == RKStatusEnumActive ? "true" : "false", vcpActive,
                    azPosition, azEnum,
                    elPosition, elEnum,
                    rate);
            RKSetHealthReady(radar, health);
        }
        usleep(200000);
    }
    return (void *)NULL;
}

#pragma mark - Protocol Implementations

RKPedestal RKPedestalPedzyInit(RKRadar *radar, void *input) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)malloc(sizeof(RKPedestalPedzy));
    if (me == NULL) {
        RKLog("Error. Unable to allocated RKPedestalPedzy.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKPedestalPedzy));
    me->radar = radar;
    
    // Pedzy uses a TCP socket server at port 9000.
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    sprintf(desc.name, "%s<PedzySmartRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPedestalRelayPedzy) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    strncpy(desc.hostname, (char *)input, RKNameLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9554;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatHeaderDefinedSize;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose =
    radar->desc.initFlags & RKInitFlagVeryVeryVerbose ? 3 :
    (radar->desc.initFlags & RKInitFlagVeryVerbose ? 2:
     (radar->desc.initFlags & RKInitFlagVerbose ? 1 : 0));

    me->client = RKClientInitWithDesc(desc);
    RKClientSetUserResource(me->client, me);
    RKClientSetGreetHandler(me->client, &RKPedestalPedzyGreet);
    RKClientSetReceiveHandler(me->client, &RKPedestalPedzyRead);
    RKClientStart(me->client, false);

    if (pthread_create(&me->tidPedestalMonitor, NULL, pedestalHealth, me) != 0) {
        RKLog("%s Error. Failed to start a pedestal monitor.\n", me->client->name);
        return (void *)RKResultFailedToStartPedestalMonitor;
    }

//    if (pthread_create(&me->tidVcpEngine, NULL, pedestalVcpEngine, me) != 0) {
//        RKLog("%s Error. Failed to start a pedestal vcp engine.\n", me->client->name);
//        return (void *)RKResultFailedToStartpedestalVcpEngine;
//    }

    return (RKPedestal)me;
}

int RKPedestalPedzyExec(RKPedestal input, const char *command, char *response) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKPositionSteerEngine *positionSteerEngine = me->radar->positionSteerEngine;
    RKClient *client = me->client;

    char cmd[16] = "";
    char cparams[4][256];
    float fparams[4];
    float az_start, az_end, az_mark, el_start, el_end, rate;
    int n;
    cparams[0][0] = '\0';
    cparams[1][0] = '\0';
    cparams[2][0] = '\0';
    cparams[3][0] = '\0';
    n = sscanf(command, "%16s %256s %256s %256s %256s", cmd, cparams[0], cparams[1], cparams[2], cparams[3]);
    n--;

    bool skipNetResponse;
    bool immediatelyDo;
    bool onlyOnce;
    immediatelyDo = false;
    onlyOnce = false;
    skipNetResponse = true;
    if (client->verbose > 1) {
        RKLog("%s Received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        if (client->verbose) {
            RKLog("%s Current client->state = 0x%08x", client->name, client->state);
        }
        if (client->state != RKClientStateConnected) {
            RKLog("%s Pedestal not connected for command '%s'.\n", client->name, command);
            if (response != NULL) {
                sprintf(response, "NAK. Pedestal not connected." RKEOL);
            }
            return RKResultClientNotConnected;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumCommandLength - 1, "%s" RKEOL, command);

        // move pedzy command here
        if (!strncmp("stop", cmd, 4) || !strncmp("zero", cmd, 4)) {
            skipNetResponse = false;
            RKPositionSteerEngineStopSweeps(positionSteerEngine);
            RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        } else if (!strncmp("go", cmd, 2) || !strncmp("run", cmd, 3)) {
            RKPositionSteerEngineArmSweeps(positionSteerEngine, RKScanRepeatForever);
            printf("ACK. Go." RKEOL);
        } else if (!strncmp("once", cmd, 4)) {
            //pedestalVcpArmSweeps(me->vcpHandle, RKPedestalVcpNoRepeat);
            RKPositionSteerEngineArmSweeps(positionSteerEngine, RKScanRepeatNone);
            printf("ACK. Once." RKEOL);
        } else if ((!strncmp("vol", cmd, 3) || !strncmp("ivol", cmd, 4) || !strncmp("ovol", cmd, 4))) {
            // vol s 10.0 10.0,90.0 20.0/p 45.0 30.0 20.0/q 0.0 30.0 10.0/r 0.0,45.0 270.0 5.0
            if (!positionSteerEngine->vcpHandle.active || !strncmp("ivol", cmd, 4)) {
                immediatelyDo = true;
                //pedestalVcpClearSweeps(positionSteerEngine->vcpHandle);
                RKPositionSteerEngineClearSweeps(positionSteerEngine);
            } else if (!strncmp("vol", cmd, 3)) {
                //pedestalVcpClearHole(positionSteerEngine->vcpHandle);
                RKPositionSteerEngineClearHole(positionSteerEngine);
            } else if (!strncmp("ovol", cmd, 4)) {
                onlyOnce = true;
                //pedestalVcpClearDeck(positionSteerEngine->vcpHandle);
                RKPositionSteerEngineClearDeck(positionSteerEngine);
            }
            const char delimiters[] = "/";
            char *token;
            char cp[strlen(command)];
            strcpy(cp, &command[4]);
            token = strtok(cp, delimiters);

            bool everythingOkay = true;
            //int VcpMode = RKVcpModeSpeedDown;
            RKScanMode mode = RKScanModeSpeedDown;
            while (token != NULL) {
                // parse in the usual way
                //                                                 elevation   azimuth     rate        ???
                n = sscanf(token, "%16s %16s %16s %16s %16s", cmd, cparams[0], cparams[1], cparams[2], cparams[3]);
                if (n < 2) {
                    printf("NAK. Ill-defined VOL." RKEOL);
                    return RKResultFailedToSetVcp;
                }
                if (sscanf(cparams[0], "%f,%f", &el_start, &el_end) == 1) {
                    el_end = el_start;
                }
                n = sscanf(cparams[1], "%f,%f,%f", &az_start, &az_end, &az_mark);
                if (n == 1) {
                    az_end = az_start;
                    az_mark = az_start;
                } else if (n == 2) {
                    az_mark = az_end;
                }
                rate = atof(cparams[2]);
                printf("EL %.2f-%.2f°   AZ %.2f-%.2f°/%.2f° @ %.2f dps\n", el_start, el_end, az_start, az_end, az_mark, rate);
                RKScanPath scan = RKPositionSteerEngineMakeScanPath(mode, el_start, el_end, az_start, az_end, az_mark, rate);

                if (*cmd == 'p') {
                    //VcpMode = RKVcpModePPIAzimuthStep;

                } else if (*cmd == 'r') {
                    //VcpMode = RKVcpModeRHI;
                    mode = RKScanModeRHI;
                } else if (*cmd == 'b') {
                    //VcpMode = RKVcpModeSpeedDown;
                    mode = RKScanModeSpeedDown;
                } else {
                    printf("NAK. Ill-defined VOL." RKEOL);
                    everythingOkay = false;
                    //pedestalVcpClearSweeps(me->vcpHandle);
                    RKPositionSteerEngineClearSweeps(positionSteerEngine);
                    return RKResultFailedToSetVcp;
                }
                if (onlyOnce) {
                    //pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(VcpMode, el_start, el_end, az_start, az_end, az_mark, rate));
                    RKPositionSteerEngineAddPinchSweep(positionSteerEngine, scan);
                } else {
                    //pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(VcpMode, el_start, el_end, az_start, az_end, az_mark, rate));
                    RKPositionSteerEngineAddLineupSweep(positionSteerEngine, scan);
                }
                token = strtok(NULL, delimiters);
            }
            if (everythingOkay) {
                //me->vcpHandle->active = true;  // already implied in NextHitter
                if (immediatelyDo) {
                    //pedestalVcpNextHitter(me->vcpHandle);
                    RKPositionSteerEngineNextHitter(positionSteerEngine);
                }
                //pedestalVcpSummary(me->vcpHandle, me->msg);
                RKPositionSteerEngineScanSummary(positionSteerEngine, me->msg);
                printf("ACK. Volume added successfully." RKEOL);
            } else {
                printf("NAK. Some error occurred." RKEOL);
            }

//        } else if ((!strncmp("pp", cmd, 2) || !strncmp("ipp", cmd, 3) || !strncmp("opp", cmd, 3))) {
//
//            char *elevations = cparams[0];
//            char *azimuth = cparams[1];
//            const char comma[] = ",";
//
//            if ( !me->vcpHandle->active || !strncmp("ipp", cmd, 3)) {
//                immediatelyDo = true;
//                pedestalVcpClearSweeps(me->vcpHandle);
//            } else if (!strncmp("pp", cmd, 2)) {
//                pedestalVcpClearHole(me->vcpHandle);
//            } else if (!strncmp("opp", cmd, 3)) {
//                onlyOnce = true;
//                pedestalVcpClearDeck(me->vcpHandle);
//            }
//
//            if (n < 1) {
//                printf("NAK. Ill-defined PPI array, n1 = %d" RKEOL, n);
//                return 1;
//            }
//            if (n == 1) {
//                az_start = 0.0f;
//                az_end = 0.0f;
//            }
//            if (n > 1) {
//                az_start = atof(azimuth);
//            } else {
//                az_start = 0.0f;
//            }
//            az_mark = az_start;
//            if (n > 2) {
//                rate = atof(cparams[2]);
//            } else {
//                rate = 18.0;
//            }
//
//            char *token = strtok(elevations, comma);
//
//            int k = 0;
//            bool everythingOkay = true;
//            while (token != NULL && k++ < 50) {
//                n = sscanf(token, "%f", &el_start);
//                if (n == 0) {
//                    everythingOkay = false;
//                    break;
//                }
//                el_end = el_start;
//                az_end = az_start;
//                if (onlyOnce) {
//                    pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModePPIAzimuthStep, el_start, el_end, az_start, az_end, az_mark, rate));
//                }else{
//                    pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModePPIAzimuthStep, el_start, el_end, az_start, az_end, az_mark, rate));
//                }
//                token = strtok(NULL, comma);
//            }
//
//            if (everythingOkay) {
//                me->vcpHandle->active = true;
//                if (immediatelyDo) {
//                    pedestalVcpNextHitter(me->vcpHandle);
//                }
//                pedestalVcpSummary(me->vcpHandle, me->msg);
//                printf("ACK. Volume added successfully." RKEOL);
//            } else {
//                printf("NAK. Some error occurred." RKEOL);
//            }
//
//        } else if ((!strncmp("rr", cmd, 2) || !strncmp("irr", cmd, 3) || !strncmp("orr", cmd, 3))) {
//
//            char *elevation = cparams[0];
//            char *azimuths = cparams[1];
//            const char comma[] = ",";
//
//            if ( !me->vcpHandle->active || !strncmp("irr", cmd, 3)) {
//                immediatelyDo = true;
//                pedestalVcpClearSweeps(me->vcpHandle);
//            } else if (!strncmp("rr", cmd, 2)) {
//                pedestalVcpClearHole(me->vcpHandle);
//            } else if (!strncmp("orr", cmd, 3)) {
//                onlyOnce = true;
//                pedestalVcpClearDeck(me->vcpHandle);
//            }
//
//            if (n < 2) {
//                printf("NAK. Ill-defined RHI array, n1 = %d" RKEOL, n);
//                return 1;
//            }
//            if (n > 2) {
//                rate = atof(cparams[2]);
//            } else {
//                rate = 18.0;
//            }
//            n = sscanf(elevation, "%f,%f", &el_start, &el_end);
//            if (n < 2) {
//                printf("NAK. Ill-defined RHI array, n2 = %d" RKEOL, n);
//                return 1;
//            }
//
//            char *token = strtok(azimuths, comma);
//
//            int k = 0;
//            bool everythingOkay = true;
//            bool RHIFlip = false;
//            while (token != NULL && k++ < 180) {
//                n = sscanf(token, "%f", &az_start);
//                if (n == 0) {
//                    everythingOkay = false;
//                    break;
//                }
//                az_end = az_start;
//                // el_start = P->limit(P->ped, el_start, INSTRUCT_MODE_POINT | INSTRUCT_AXIS_EL);
//                // el_end = P->limit(P->ped, el_end, INSTRUCT_MODE_POINT | INSTRUCT_AXIS_EL);
//                // az_start = P->limit(P->ped, az_start, INSTRUCT_MODE_POINT | INSTRUCT_AXIS_AZ);
//                // az_end = az_start;
//                // rate = P->limit(P->ped, rate, INSTRUCT_MODE_SLEW | INSTRUCT_AXIS_EL);
//                if (onlyOnce) {
//                    if (RHIFlip) {
//                        pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_end, el_start, az_start, az_end, 0.0f, -rate));
//                    } else {
//                        pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_start, el_end, az_start, az_end, 0.0f, rate));
//                    }
//                }else{
//                    if (RHIFlip) {
//                        pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_end, el_start, az_start, az_end, 0.0f, -rate));
//                    } else {
//                        pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_start, el_end, az_start, az_end, 0.0f, rate));
//                    }
//                }
//                RHIFlip = !RHIFlip;
//                token = strtok(NULL, comma);
//            }
//            if (everythingOkay) {
//                me->vcpHandle->active = true;
//                if (immediatelyDo) {
//                    pedestalVcpNextHitter(me->vcpHandle);
//                }
//                pedestalVcpSummary(me->vcpHandle, me->msg);
//                printf("ACK. Volume added successfully." RKEOL);
//            } else {
//                printf("NAK. Some error occurred." RKEOL);
//            }
//
//        } else if (!strncmp("summ", cmd, 4)) {
//            pedestalVcpSummary(me->vcpHandle, me->msg);
//            strncpy(response, me->msg, RKMaximumStringLength - 1);
//
//        } else if (!strncmp("spoint", cmd, 6)) {
//            // Point
//            if (n < 1) {
//                sprintf(me->msg, "NAK. Use as: %s [AZ] [EL]" RKEOL, cmd);
//                strncpy(response, me->msg, RKMaximumStringLength - 1);
//                return RKResultFailedToExecuteCommand;
//            }
//            fparams[0] = atof(cparams[0]);
//            if (n > 1) {
//                fparams[1] = atof(cparams[1]);
//                snprintf(me->msg, RKMaximumStringLength - 1, "ACK. Pointing AZ:%.2f deg, EL:%.2f deg." RKEOL,
//                         fparams[0],
//                         fparams[1]
//                         );
//            } else {
//                RKPosition *pos = RKGetLatestPosition(me->radar);
//                fparams[1] = pos->elevationDegrees;
//                snprintf(me->msg, RKMaximumStringLength - 1, "ACK. Pointing AZ:%.2f deg." RKEOL,
//                         fparams[0]
//                         );
//            }
//            pedestalPoint(me, fparams[1], fparams[0]);
//            strncpy(response, me->msg, RKMaximumStringLength - 1);
//
        } else {
            skipNetResponse = false;
            RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        }

        if (!skipNetResponse){
            while (responseIndex == me->responseIndex) {
                usleep(10000);
                if (++s % 100 == 0) {
                    RKLog("%s Waited %.2f s for response to '%s'.\n", client->name, (float)s * 0.01f, command);
                }
                if ((float)s * 0.01f >= 3.0f) {
                    RKLog("%s should time out.\n", client->name);
                    break;
                }
            }
            if (responseIndex == me->responseIndex) {
                if (response != NULL) {
                    sprintf(response, "NAK. Timeout." RKEOL);
                }
                return RKResultTimeout;
            }
            if (response != NULL) {
                strcpy(response, me->responses[responseIndex]);
            }
        }
    }
    return RKResultSuccess;
}

int RKPedestalPedzyFree(RKPedestal input) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultSuccess;
}

//
//
//
//
//void pedestalVcpSendAction(int sd, char *ship, RKPedestalAction *act) {
//    sprintf(ship, "%c", 0x0f);
//    memcpy(ship + 1, act, sizeof(RKPedestalAction));
//    strncpy(ship + 1 + sizeof(RKPedestalAction), RKEOL, 2);
//    // printf("EL %.2f dps| AZ %.2f dps \n",act->param[0],act->param[1]);
//    RKNetworkSendPackets(sd, ship, sizeof(RKPedestalAction) + 3, NULL);
//}
//

//
//
//
//int pedestalPoint(RKPedestalPedzy *me, const float el_point, const float az_point){
//    float umin_diff_el;
//    float umin_diff_az;
//    float min_diff_el;
//    float min_diff_az;
//    float rate_el;
//    float rate_az;
//    RKPosition *pos = RKGetLatestPosition(me->radar);
//    RKPedestalAction action;
//    action.mode[0] = RKPedestalInstructTypeNone;
//    action.mode[1] = RKPedestalInstructTypeNone;
//    action.param[0] = 0.0f;
//    action.param[1] = 0.0f;
//    action.sweepElevation = el_point;
//    action.sweepAzimuth = az_point;
//
//    umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//    umin_diff_az = RKUMinDiff(az_point, pos->azimuthDegrees);
//    int i = 0;
//    while ((umin_diff_el > RKPedestalPositionRoom || umin_diff_az > RKPedestalPositionRoom)
//        && i < RKPedestalPointTimeOut) {
//
//        pos = RKGetLatestPosition(me->radar);
//        umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//        if (umin_diff_el > RKPedestalPositionRoom){
//            action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//            rate_el = pedestalGetRate( umin_diff_el, RKPedestalPointElevation );
//            min_diff_el = RKMinDiff(el_point, pos->elevationDegrees);
//            if (min_diff_el >= 0.0f){
//                action.param[0] = rate_el;
//            }else{
//                action.param[0] = -rate_el;
//            }
//        } else{
//            action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
//            action.param[0] = 0.0f;
//        }
//
//        umin_diff_az = RKUMinDiff(az_point, pos->azimuthDegrees);
//        if (umin_diff_az > RKPedestalPositionRoom){
//            action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//            rate_az = pedestalGetRate( umin_diff_az, RKPedestalPointAzimuth );
//            min_diff_az = RKMinDiff(az_point, pos->azimuthDegrees);
//            if (min_diff_az >= 0.0f){
//                action.param[1] = rate_az;
//            }else{
//                action.param[1] = -rate_az;
//            }
//        } else{
//            action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
//            action.param[1] = 0.0f;
//        }
//        // printf("EL point %.2f | AZ point %.2f \n",pos->elevationDegrees,pos->azimuthDegrees);
//        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//        i++;
//        usleep(20000);
//    }
//    action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
//    action.param[0] = 0.0f;
//    action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
//    action.param[1] = 0.0f;
//    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//    printf("%s","Point finish.");
//    if (i < RKPedestalPointTimeOut){
//        return 0;
//    }else{
//        return 1;
//    }
//}
//
//int pedestalAzimuthPoint(RKPedestalPedzy *me, const float az_point, const float rate_el){
//    float umin_diff_az;
//    float min_diff_az;
//    float rate_az;
//    RKPosition *pos = RKGetLatestPosition(me->radar);
//    RKPedestalAction action;
//    action.mode[0] = RKPedestalInstructTypeNone;
//    action.mode[1] = RKPedestalInstructTypeNone;
//    action.param[0] = 0.0f;
//    action.param[1] = 0.0f;
//    action.sweepElevation = pos->elevationDegrees;
//    action.sweepAzimuth = az_point;
//    umin_diff_az = RKUMinDiff(az_point, pos->azimuthDegrees);
//    int i = 0;
//    while (umin_diff_az > RKPedestalPositionRoom && i < RKPedestalPointTimeOut) {
//        pos = RKGetLatestPosition(me->radar);
//        umin_diff_az = RKUMinDiff(az_point, pos->azimuthDegrees);
//        if (umin_diff_az > RKPedestalPositionRoom){
//            action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//            rate_az = pedestalGetRate( umin_diff_az, RKPedestalPointAzimuth );
//            min_diff_az = RKMinDiff(az_point, pos->azimuthDegrees);
//            if (min_diff_az >= 0.0f){
//                action.param[1] = rate_az;
//            }else{
//                action.param[1] = -rate_az;
//            }
//        } else{
//            action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
//            action.param[1] = 0.0f;
//        }
//        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//        action.param[0] = rate_el;
//        // printf("EL slew %.2f | AZ point %.2f dps\n",rate_el,pos->azimuthDegrees);
//        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//        i++;
//        usleep(20000);
//    }
//    action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//    action.param[0] = rate_el;
//    action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
//    action.param[1] = 0.0f;
//    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//    printf("%s","Azimuth point finish.");
//    if (i < RKPedestalPointTimeOut){
//        return 0;
//    }else{
//        return 1;
//    }
//}
//
//int pedestalElevationPoint(RKPedestalPedzy *me, const float el_point, const float rate_az){
//    float umin_diff_el;
//    float min_diff_el;
//    float rate_el;
//    RKPosition *pos = RKGetLatestPosition(me->radar);
//    RKPedestalAction action;
//    action.mode[0] = RKPedestalInstructTypeNone;
//    action.mode[1] = RKPedestalInstructTypeNone;
//    action.param[0] = 0.0f;
//    action.param[1] = 0.0f;
//    action.sweepElevation = el_point;
//    action.sweepAzimuth = pos->azimuthDegrees;
//    umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//    int i = 0;
//    while (umin_diff_el > RKPedestalPositionRoom && i < RKPedestalPointTimeOut) {
//
//        pos = RKGetLatestPosition(me->radar);
//        umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//        if (umin_diff_el > RKPedestalPositionRoom){
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
//    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//
//    printf("%s","Elevation point finish.");
//    if (i < RKPedestalPointTimeOut){
//        return 0;
//    }else{
//        return 1;
//    }
//}
//
//
//RKPedestalAction pedestalAzimuthPointNudge(RKPedestalPedzy *me, const float az_point, const float rate_el){
//    float umin_diff_az;
//    float min_diff_az;
//    float rate_az;
//    RKPosition *pos = RKGetLatestPosition(me->radar);
//    RKPedestalAction action;
//    action.mode[0] = RKPedestalInstructTypeNone;
//    action.mode[1] = RKPedestalInstructTypeNone;
//    action.param[0] = 0.0f;
//    action.param[1] = 0.0f;
//    action.sweepElevation = pos->elevationDegrees;
//    action.sweepAzimuth = az_point;
//    umin_diff_az = RKUMinDiff(az_point, pos->azimuthDegrees);
//    if (umin_diff_az > RKPedestalPositionRoom){
//        action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//        rate_az = pedestalGetRate( umin_diff_az, RKPedestalPointAzimuth );
//        min_diff_az = RKMinDiff(az_point, pos->azimuthDegrees);
//        if (min_diff_az >= 0.0f){
//            action.param[1] = rate_az;
//        }else{
//            action.param[1] = -rate_az;
//        }
//    } else{
//        action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
//        action.param[1] = 0.0f;
//    }
//    action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//    action.param[0] = rate_el;
//    return action;
//}
//
//
//RKPedestalAction pedestalElevationPointNudge(RKPedestalPedzy *me, const float el_point, const float rate_az){
//    float umin_diff_el;
//    float min_diff_el;
//    float rate_el;
//    RKPosition *pos = RKGetLatestPosition(me->radar);
//    RKPedestalAction action;
//    action.mode[0] = RKPedestalInstructTypeNone;
//    action.mode[1] = RKPedestalInstructTypeNone;
//    action.param[0] = 0.0f;
//    action.param[1] = 0.0f;
//    action.sweepElevation = el_point;
//    action.sweepAzimuth = pos->azimuthDegrees;
//    umin_diff_el = RKUMinDiff(el_point, pos->elevationDegrees);
//    if (umin_diff_el > RKPedestalPositionRoom){
//        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//        rate_el = pedestalGetRate( umin_diff_el, RKPedestalPointElevation );
//        min_diff_el = RKMinDiff(el_point, pos->elevationDegrees);
//        if (min_diff_el >= 0.0f){
//            action.param[0] = rate_el;
//        }else{
//            action.param[0] = -rate_el;
//        }
//    } else{
//        action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
//        action.param[0] = 0.0f;
//    }
//    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//    action.param[1] = rate_az;
//    return action;
//}
//
//int pedestalSlowDown(RKPedestalPedzy *me){
//    float umin_diff_vel_el;
//    float umin_diff_vel_az;
//    RKPosition *pos = RKGetLatestPosition(me->radar);
//    RKPedestalAction action;
//
//    umin_diff_vel_el = RKUMinDiff(0.0f, pos->elevationVelocityDegreesPerSecond);
//    umin_diff_vel_az = RKUMinDiff(0.0f, pos->azimuthVelocityDegreesPerSecond);
//    int tic = 0;
//    while ((umin_diff_vel_el > RKPedestalVelocityRoom || umin_diff_vel_az > RKPedestalVelocityRoom) && tic < RKPedestalPointTimeOut) {
//        pos = RKGetLatestPosition(me->radar);
//        umin_diff_vel_el = RKUMinDiff(0.0f, pos->elevationVelocityDegreesPerSecond);
//        umin_diff_vel_az = RKUMinDiff(0.0f, pos->azimuthVelocityDegreesPerSecond);
//        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
//        action.param[0] = pos->elevationVelocityDegreesPerSecond * 0.8;
//        action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
//        action.param[1] = pos->azimuthVelocityDegreesPerSecond * 0.8;
//        printf("EL slew %.2f | AZ slew %.2f dps\n",
//            pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
//        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//        tic++;
//        usleep(20000);
//    }
//    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
//    action.param[0] = 0;
//    action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
//    action.param[1] = 0;
//    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
//    printf("%s","SlowDown finish.");
//    if (tic < RKPedestalPointTimeOut) {
//        return 0;
//    } else {
//        return 1;
//    }
//}
