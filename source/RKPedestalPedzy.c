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

    //RKPedestalVcpHandle *vcpHandle = &radar->positionSteerEngine->vcpHandle;
    // RKPositionFlag flag;
    RKPositionSteerEngine *steerEngine = radar->positionSteerEngine;

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

        // Add other flags based on radar->positionSteerEngine
        //RKPositionSteerEngineUpdatePositionFlags(steerEngine, newPosition);

        // Note to myself: Could consider adding something similar to the RKRadar abstraction layer
        // That way, the thought process is consistent like RKSetPositionRead();
        //
        // RKUpdatePositionFlags(radar, newPosition);
        //

        RKSetPositionReady(radar, newPosition);

        // Get the latest action, could be null
        // Same here, add one to the RKRadar layer?
        RKScanAction *action = RKPositionSteerEngineGetAction(steerEngine, newPosition);

        char axis = 'e';
        char string[64];
        for (int k = 0; k < 2; k++) {
            RKPedestalInstructType instruct = action->mode[k];
            float value = action->param[k];
            if (instruct == 0) {
                continue;
            }
            if (RKInstructIsElevation(instruct)) {
                axis = 'e';
            } else {
                axis = 'a';
            }
            if (RKInstructIsSlew(instruct)) {
                sprintf(string, "%cslew %.2f" RKEOL, axis, value);
            } else if (RKInstructIsPoint(instruct)) {
                sprintf(string, "%cpoint %.2f" RKEOL, axis, value);
            } else if (RKInstructIsStandby(instruct)) {
                sprintf(string, "%cstop" RKEOL, axis);
            }
            printf("string = '%s'\n", string);
            // RKPedestalPedzyExec(me, string, response);
        }

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
//                               RKInstructIsAzimuth(action.mode[i]) ? "AZ"       : "EL",
//                               RKInstructIsPoint(action.mode[i])   ? " point"   : "",
//                               RKInstructIsSlew(action.mode[i])    ? " slew"    : "",
//                               RKInstructIsStandby(action.mode[i]) ? " standby" : "",
//                               RKInstructIsEnable(action.mode[i])  ? " enable"  : "",
//                               RKInstructIsDisable(action.mode[i]) ? " disable" : "",
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
    RKPositionSteerEngine *steerer = radar->positionSteerEngine;
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
            } else if (steerer->vcpHandle.active) {
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
            sprintf(health->string, "{"
                    "\"Pedestal AZ Interlock\":{\"Value\":%s,\"Enum\":%d}, "
                    "\"Pedestal EL Interlock\":{\"Value\":%s,\"Enum\":%d}, "
                    "\"VCP Active\":{\"Value\":%s,\"Enum\":%d}, "
                    "\"Pedestal AZ\":{\"Value\":\"%s\",\"Enum\":%d}, "
                    "\"Pedestal EL\":{\"Value\":\"%s\",\"Enum\":%d}, "
                    "\"Pedestal Update\":\"%.3f Hz\", "
                    "\"PedestalHealthEnd\":0"
                    "}",
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

    bool skipNetResponse = true;

    if (response == NULL) {
        response = (char *)me->dump;
    }

    if (client->verbose > 1) {
        RKLog("%s Received '%s'", client->name, command);
    }

    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else if (!strncmp("go", command, 2) || !strncmp("run", command, 3)) {
        RKPositionSteerEngineArmSweeps(positionSteerEngine, RKScanRepeatForever);
        sprintf(response, "ACK. Go." RKEOL);
    } else if (!strncmp("once", command, 4)) {
        RKPositionSteerEngineArmSweeps(positionSteerEngine, RKScanRepeatNone);
        sprintf(response, "ACK. Once." RKEOL);
    } else if (!strncmp("pp", command, 2) ||
                !strncmp("ipp", command, 3) ||
                !strncmp("opp", command, 3) ||
                !strncmp("rr", command, 2) ||
                !strncmp("irr", command, 3) ||
                !strncmp("orr", command, 3) ||
                !strncmp("vol", command, 3) ||
                !strncmp("ivol", command, 4) ||
                !strncmp("ovol", command, 4)) {
        RKPositionSteerEngineExecuteString(positionSteerEngine, command, response);
    } else if (!strncmp("summ", command, 4)) {
        RKPositionSteerEngineScanSummary(positionSteerEngine, response);
        sprintf(response + strlen(response), "ACK. Summary retrieved" RKEOL);
    } else {
        if (client->verbose) {
            RKLog("%s Current client->state = 0x%08x", client->name, client->state);
        }
        if (client->state != RKClientStateConnected) {
            RKLog("%s Pedestal not connected for command '%s'.\n", client->name, command);
            sprintf(response, "NAK. Pedestal not connected." RKEOL);
            return RKResultClientNotConnected;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumCommandLength - 1, "%s" RKEOL, command);

        // Commands that need to be forwarded to Pedzy
        if (!strncmp("stop", command, 4) || !strncmp("zero", command, 4)) {
            skipNetResponse = false;
            RKPositionSteerEngineStopSweeps(positionSteerEngine);
            RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        } else {
            skipNetResponse = false;
            RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        }

        if (!skipNetResponse) {
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
                sprintf(response, "NAK. Timeout." RKEOL);
                return RKResultTimeout;
            }
            strcpy(response, me->responses[responseIndex]);
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
