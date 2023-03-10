//
//  RKPedestalPedzy.c
//  RadarKit
//
//  Created by Boonleng Cheong on 1/4/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPedestalPedzy.h>

// Internal Functions

static int RKPedestalPedzyRead(RKClient *);
static int RKPedestalPedzyGreet(RKClient *);
static void *pedestalHealth(void *);

RKPedestalAction pedestalVcpGetAction(RKPedestalPedzy *);
void pedestalVcpArmSweeps(RKPedestalVcpHandle *, const bool);
void pedestalVcpClearSweeps(RKPedestalVcpHandle *);
void pedestalVcpClearHole(RKPedestalVcpHandle *);
void pedestalVcpClearDeck(RKPedestalVcpHandle *);
void pedestalVcpNextHitter(RKPedestalVcpHandle *);
RKPedestalVcpSweepHandle pedestalVcpMakeSweep(RKVcpMode mode,
                       const float el_start, const float el_end,
                       const float az_start, const float az_end, const float az_mark,
                       const float rate);
int pedestalVcpAddLineupSweep(RKPedestalVcpHandle *, RKPedestalVcpSweepHandle sweep);
int pedestalVcpAddPinchSweep(RKPedestalVcpHandle *, RKPedestalVcpSweepHandle sweep);
void makeSweepMessage(RKPedestalVcpSweepHandle *, char *, int SC, RKVcpHitter linetag);
float pedestalGetRate(const float diff_deg, int axis);
int pedestalPoint(RKPedestalPedzy *, const float el_point, const float az_point);
int pedestalAzimuthPoint(RKPedestalPedzy *, const float az_point, const float rate_el);
int pedestalElevationPoint(RKPedestalPedzy *, const float el_point, const float rate_az);
RKPedestalAction pedestalAzimuthPointNudge(RKPedestalPedzy *, const float az_point, const float rate_el);
RKPedestalAction pedestalElevationPointNudge(RKPedestalPedzy *, const float el_point, const float rate_az);
void pedestalVcpSummary(RKPedestalVcpHandle *, char *);

float min_diff(const float v1, const float v2) {
    float d = v1 - v2;
    if (d >= 180.0f) {
        d -= 360.0f;
    } else if (d < -180.0f) {
        d += 360.0f;
    }
    return d;
}

float umin_diff(const float v1, const float v2) {
    float d = v1 - v2;
    if (d >= 180.0f) {
        d -= 360.0f;
    } else if (d < -180.0f) {
        d += 360.0f;
    }
    if (d < 0.0f) {
        return -d;
    }
    return d;
}

#pragma mark - Internal Functions

static int RKPedestalPedzyRead(RKClient *client) {
    // The shared user resource pointer
    RKPedestalPedzy *me = (RKPedestalPedzy *)client->userResource;
    RKRadar *radar = me->radar;

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
        if (me->vcpHandle->active){
            int i = me->vcpHandle->i;
            radar->positionEngine->vcpI = i;
            radar->positionEngine->vcpSweepCount = me->vcpHandle->sweepCount;
            newPosition->sweepElevationDegrees = me->vcpHandle->sweepMarkerElevation;
            newPosition->sweepAzimuthDegrees = me->vcpHandle->sweepAzimuth;

            // clear flag
            // newPosition->flag = RKPositionFlagVacant;
            // VCP engine is active
            newPosition->flag |= RKPositionFlagVCPActive;
            // Sweep is active: data collection portion (i.e., exclude transitions)
            if (me->vcpHandle->progress & RKVcpProgressReady ||
                me->vcpHandle->progress & RKVcpProgressMiddle ||
                me->vcpHandle->batterSweeps[i].mode == RKVcpModePPIAzimuthStep ||
                me->vcpHandle->batterSweeps[i].mode == RKVcpModePPIContinuous) {
                // Always active if any of the conditions above are met
                newPosition->flag |= RKPositionFlagScanActive;
            }
            // Point / sweep flag
            switch (me->vcpHandle->batterSweeps[i].mode) {
                case RKVcpModePPI:
                case RKVcpModeSector:
                case RKVcpModeNewSector:
                case RKVcpModePPIAzimuthStep:
                case RKVcpModePPIContinuous:
                    newPosition->flag |= RKPositionFlagAzimuthSweep;
                    newPosition->flag |= RKPositionFlagElevationPoint;
                    //pos->flag &= ~POSITION_FLAG_EL_SWEEP;
                    break;
                case RKVcpModeRHI:
                    newPosition->flag |= RKPositionFlagAzimuthPoint;
                    newPosition->flag |= RKPositionFlagElevationSweep;
                    //pos->flag &= ~POSITION_FLAG_AZ_SWEEP;
                    break;
                default:
                    newPosition->flag |= RKPositionFlagAzimuthPoint | RKPositionFlagElevationPoint;
                    break;
            }
            // Completion flag
            if (me->vcpHandle->progress & RKVcpProgressMarker) {
                me->vcpHandle->progress ^= RKVcpProgressMarker;
                switch (me->vcpHandle->batterSweeps[i].mode) {
                    case RKVcpModePPI:
                    case RKVcpModeSector:
                    case RKVcpModeNewSector:
                    case RKVcpModePPIAzimuthStep:
                    case RKVcpModePPIContinuous:
                        newPosition->flag |= RKPositionFlagAzimuthComplete;
                        break;
                    case RKVcpModeRHI:
                        newPosition->flag |= RKPositionFlagElevationComplete;
                        break;
                    default:
                        break;
                }
            }
        } else{
            radar->positionEngine->vcpI = 0;
            radar->positionEngine->vcpSweepCount = 0;
        }
        RKSetPositionReady(radar, newPosition);
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
static void *pedestalVcpEngine(void *in) {
    int i;
    RKPedestalPedzy *me = (RKPedestal)in;
    RKRadar *radar = me->radar;
    RKClient *client = me->client;
    RKStatusEnum azInterlockStatus = RKStatusEnumInvalid;
    RKStatusEnum elInterlockStatus = RKStatusEnumInvalid;
    RKStatusEnum vcpActive;

    gettimeofday(&RKPedestalVcpCurrentTime, NULL);
    timerclear(&RKPedestalVcpStatusTriggerTime);
    timerclear(&RKPedestalVcpStatusPeriod);
    RKPedestalVcpStatusPeriod.tv_sec = RKPedestalVcpStatusPeriodMS / 1000;
    RKPedestalVcpStatusPeriod.tv_usec = (RKPedestalVcpStatusPeriodMS % 1000) * 1000;
    timeradd(&RKPedestalVcpCurrentTime, &RKPedestalVcpStatusPeriod, &RKPedestalVcpStatusTriggerTime);
    // RKPedestalVcpHandle *V = me->vcpHandle;
    RKStatusEnum azEnum;
    RKStatusEnum elEnum;
    RKPedestalAction action;
    while (me->client->state < RKClientStateDisconnecting) {
        gettimeofday(&RKPedestalVcpCurrentTime, NULL);
        if (me->client->state < RKClientStateConnected) {
            azInterlockStatus = RKStatusEnumInvalid;
            elInterlockStatus = RKStatusEnumInvalid;
            vcpActive = RKStatusEnumInvalid;
            printf("--.-- deg");
            printf("--.-- deg");
            // sprintf(elPosition, "--.-- deg");
            // sprintf(azPosition, "--.-- deg");
            azEnum = RKStatusEnumInvalid;
            elEnum = RKStatusEnumInvalid;
        } else {
            RKPosition *position = RKGetLatestPosition(radar);
            azInterlockStatus = position->flag & RKPositionFlagAzimuthSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            elInterlockStatus = position->flag & RKPositionFlagElevationSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            if (position->flag & (RKPositionFlagAzimuthError | RKPositionFlagElevationError)) {
                vcpActive = RKStatusEnumFault;
            } else if (position->flag & RKPositionFlagVCPActive) {
                vcpActive = RKStatusEnumActive;
            } else {
                vcpActive = RKStatusEnumStandby;
            }
            // printf("%.2f deg", position->elevationDegrees);
            // printf("%.2f deg", position->azimuthDegrees);
            // sprintf(elPosition, "%.2f deg", position->elevationDegrees);
            // sprintf(azPosition, "%.2f deg", position->azimuthDegrees);
            azEnum = RKStatusEnumNormal;
            elEnum = RKStatusEnumNormal;

            if (me->vcpHandle->active){
                // put vcp get action here
                action = pedestalVcpGetAction(me);
                // action.sweepElevation = me->vcpHandle->batterSweeps[me->vcpHandle->i].elevationStart;
                // action.sweepAzimuth = me->vcpHandle->batterSweeps[me->vcpHandle->i].azimuthStart;
                me->vcpHandle->lastAction = action;
                // Call the control handler based on the suggested action
                if (action.mode[0] != RKPedestalInstructTypeNone || action.mode[1] != RKPedestalInstructTypeNone) {
                    // P->control(P->ped, action);
                    pedestalVcpSendAction(client->sd, me->latestCommand, &action);

                    for (i = 0; i < 2; i++) {
                        if (action.mode[i] != RKPedestalInstructTypeNone) {
                            RKLog("action.mode[%d] %s%s%s%s%s%s %.2f\n", i,
                               InstructIsAzimuth(action.mode[i]) ? "AZ"       : "EL",
                               InstructIsPoint(action.mode[i])   ? " point"   : "",
                               InstructIsSlew(action.mode[i])    ? " slew"    : "",
                               InstructIsStandby(action.mode[i]) ? " standby" : "",
                               InstructIsEnable(action.mode[i])  ? " enable"  : "",
                               InstructIsDisable(action.mode[i]) ? " disable" : "",
                               action.param[i]);
                        }
                    }
                    me->lastActionAge = 0;
                }
            }
            if (timercmp(&RKPedestalVcpCurrentTime, &RKPedestalVcpStatusTriggerTime, >=)) {
                timeradd(&RKPedestalVcpCurrentTime, &RKPedestalVcpStatusPeriod, &RKPedestalVcpStatusTriggerTime);
                if (me->vcpHandle->active) {
                    printf("                 -- [ VCP sweep %d / %d -- elevation = %.2f -- azimuth = %.2f -- prog %d ] --\n",
                        me->vcpHandle->i,
                        me->vcpHandle->sweepCount,
                        position->elevationDegrees,
                        position->azimuthDegrees,
                        me->vcpHandle->progress);
                } else {
                    printf("                 -- [ VCP inactive ] --\n");
                }
            }
        }
        usleep(20000);
        me->lastActionAge++;
    }
    return (void *)NULL;
}

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
            azInterlockStatus = position->flag & RKPositionFlagAzimuthSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            elInterlockStatus = position->flag & RKPositionFlagElevationSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            if (position->flag & (RKPositionFlagAzimuthError | RKPositionFlagElevationError)) {
                vcpActive = RKStatusEnumFault;
            } else if (position->flag & RKPositionFlagVCPActive) {
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
    me->vcpHandle = pedestalVcpInit();
    if (me->vcpHandle == NULL) {
        exit(EXIT_FAILURE);
    }
    RKClientSetUserResource(me->client, me);
    RKClientSetGreetHandler(me->client, &RKPedestalPedzyGreet);
    RKClientSetReceiveHandler(me->client, &RKPedestalPedzyRead);
    RKClientStart(me->client, false);

    if (pthread_create(&me->tidPedestalMonitor, NULL, pedestalHealth, me) != 0) {
        RKLog("%s Error. Failed to start a pedestal monitor.\n", me->client->name);
        return (void *)RKResultFailedToStartPedestalMonitor;
    }

    if (pthread_create(&me->tidVcpEngine, NULL, pedestalVcpEngine, me) != 0) {
        RKLog("%s Error. Failed to start a pedestal vcp engine.\n", me->client->name);
        return (void *)RKResultFailedToStartpedestalVcpEngine;
    }

    return (RKPedestal)me;
}

int RKPedestalPedzyExec(RKPedestal input, const char *command, char *response) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClient *client = me->client;

    char cmd[16] = "";
    char cparams[4][256];
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
            me->vcpHandle->active = false;
            RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        } else if (!strncmp("go", cmd, 2) || !strncmp("run", cmd, 3)) {
            pedestalVcpArmSweeps(me->vcpHandle,RKPedestalVcpRepeat);
            printf("ACK. Go." RKEOL);
        } else if (!strncmp("once", cmd, 4)) {
            pedestalVcpArmSweeps(me->vcpHandle, RKPedestalVcpNoRepeat);
            printf("ACK. Once." RKEOL);
        } else if ((!strncmp("vol", cmd, 3) || !strncmp("lvol", cmd, 4) || !strncmp("ovol", cmd, 4))) {
            // vol s 10.0 10.0,90.0 20.0/p 45.0 30.0 20.0/q 0.0 30.0 10.0/r 0.0,45.0 270.0 5.0
            if (!strncmp("vol", cmd, 3)) {
                immediatelyDo = true;
                pedestalVcpClearSweeps(me->vcpHandle);
            } else if (!strncmp("lvol", cmd, 4)) {
                pedestalVcpClearHole(me->vcpHandle);
            } else if (!strncmp("ovol", cmd, 4)) {
                onlyOnce = true;
                pedestalVcpClearDeck(me->vcpHandle);
            }
            const char delimiters[] = "/";
            char *token;
            char cp[strlen(command)];
            strcpy(cp, &command[4]);
            token = strtok(cp, delimiters);

            bool everythingOkay = true;
            int VcpMode = RKVcpModeSpeedDown;
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

                if (*cmd == 'p') {
                    VcpMode = RKVcpModePPIAzimuthStep;
                } else if (*cmd == 'r') {
                    VcpMode = RKVcpModeRHI;
                } else if (*cmd == 'b') {
                    VcpMode = RKVcpModeSpeedDown;
                } else {
                    printf("NAK. Ill-defined VOL." RKEOL);
                    everythingOkay = false;
                    pedestalVcpClearSweeps(me->vcpHandle);
                    return RKResultFailedToSetVcp;
                }
                if (onlyOnce) {
                    pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(VcpMode, el_start, el_end, az_start, az_end, az_mark, rate));
                }else{
                    pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(VcpMode, el_start, el_end, az_start, az_end, az_mark, rate));
                }
                token = strtok(NULL, delimiters);
            }
            if (everythingOkay) {
                me->vcpHandle->active = true;
                if (immediatelyDo) {
                    pedestalVcpNextHitter(me->vcpHandle);
                }

                pedestalVcpSummary(me->vcpHandle, me->msg);
                printf("ACK. Volume added successfully." RKEOL);
            } else {
                printf("NAK. Some error occurred." RKEOL);
            }

        } else if ((!strncmp("pp", cmd, 2) || !strncmp("lpp", cmd, 3) || !strncmp("opp", cmd, 3))) {

            char *elevations = cparams[0];
            char *azimuth = cparams[1];
            const char comma[] = ",";

            if (!strncmp("pp", cmd, 2)) {
                immediatelyDo = true;
                pedestalVcpClearSweeps(me->vcpHandle);
            } else if (!strncmp("lpp", cmd, 3)) {
                pedestalVcpClearHole(me->vcpHandle);
            } else if (!strncmp("opp", cmd, 3)) {
                onlyOnce = true;
                pedestalVcpClearDeck(me->vcpHandle);
            }

            if (n < 1) {
                printf("NAK. Ill-defined PPI array, n1 = %d" RKEOL, n);
                return 1;
            }
            if (n == 1) {
                az_start = 0.0f;
                az_end = 0.0f;
            }
            if (n > 1) {
                az_start = atof(azimuth);
            } else {
                az_start = 0.0f;
            }
            az_mark = az_start;
            if (n > 2) {
                rate = atof(cparams[2]);
            } else {
                rate = 18.0;
            }

            char *token = strtok(elevations, comma);

            int k = 0;
            bool everythingOkay = true;
            while (token != NULL && k++ < 50) {
                n = sscanf(token, "%f", &el_start);
                if (n == 0) {
                    everythingOkay = false;
                    break;
                }
                el_end = el_start;
                az_end = az_start;
                if (onlyOnce) {
                    pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModePPIAzimuthStep, el_start, el_end, az_start, az_end, az_mark, rate));
                }else{
                    pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModePPIAzimuthStep, el_start, el_end, az_start, az_end, az_mark, rate));
                }
                token = strtok(NULL, comma);
            }

            if (everythingOkay) {
                me->vcpHandle->active = true;
                if (immediatelyDo) {
                    pedestalVcpNextHitter(me->vcpHandle);
                }
                pedestalVcpSummary(me->vcpHandle, me->msg);
                printf("ACK. Volume added successfully." RKEOL);
            } else {
                printf("NAK. Some error occurred." RKEOL);
            }

        } else if ((!strncmp("rr", cmd, 2) || !strncmp("lrr", cmd, 3) || !strncmp("orr", cmd, 3))) {

            char *elevation = cparams[0];
            char *azimuths = cparams[1];
            const char comma[] = ",";

            if (!strncmp("rr", cmd, 2)) {
                immediatelyDo = true;
                pedestalVcpClearSweeps(me->vcpHandle);
            } else if (!strncmp("lrr", cmd, 3)) {
                pedestalVcpClearHole(me->vcpHandle);
            } else if (!strncmp("orr", cmd, 3)) {
                onlyOnce = true;
                pedestalVcpClearDeck(me->vcpHandle);
            }

            if (n < 2) {
                printf("NAK. Ill-defined RHI array, n1 = %d" RKEOL, n);
                return 1;
            }
            if (n > 2) {
                rate = atof(cparams[2]);
            } else {
                rate = 18.0;
            }
            n = sscanf(elevation, "%f,%f", &el_start, &el_end);
            if (n < 2) {
                printf("NAK. Ill-defined RHI array, n2 = %d" RKEOL, n);
                return 1;
            }

            char *token = strtok(azimuths, comma);

            int k = 0;
            bool everythingOkay = true;
            bool RHIFlip = false;
            while (token != NULL && k++ < 180) {
                n = sscanf(token, "%f", &az_start);
                if (n == 0) {
                    everythingOkay = false;
                    break;
                }
                az_end = az_start;
                // el_start = P->limit(P->ped, el_start, INSTRUCT_MODE_POINT | INSTRUCT_AXIS_EL);
                // el_end = P->limit(P->ped, el_end, INSTRUCT_MODE_POINT | INSTRUCT_AXIS_EL);
                // az_start = P->limit(P->ped, az_start, INSTRUCT_MODE_POINT | INSTRUCT_AXIS_AZ);
                // az_end = az_start;
                // rate = P->limit(P->ped, rate, INSTRUCT_MODE_SLEW | INSTRUCT_AXIS_EL);
                if (onlyOnce) {
                    if (RHIFlip) {
                        pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_end, el_start, az_start, az_end, 0.0f, -rate));
                    } else {
                        pedestalVcpAddPinchSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_start, el_end, az_start, az_end, 0.0f, rate));
                    }
                }else{
                    if (RHIFlip) {
                        pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_end, el_start, az_start, az_end, 0.0f, -rate));
                    } else {
                        pedestalVcpAddLineupSweep(me->vcpHandle, pedestalVcpMakeSweep(RKVcpModeRHI, el_start, el_end, az_start, az_end, 0.0f, rate));
                    }
                }
                RHIFlip = !RHIFlip;
                token = strtok(NULL, comma);
            }
            if (everythingOkay) {
                me->vcpHandle->active = true;
                if (immediatelyDo) {
                    pedestalVcpNextHitter(me->vcpHandle);
                }
                pedestalVcpSummary(me->vcpHandle, me->msg);
                printf("ACK. Volume added successfully." RKEOL);
            } else {
                printf("NAK. Some error occurred." RKEOL);
            }

        } else if (!strncmp("summ", cmd, 4)) {
            pedestalVcpSummary(me->vcpHandle, me->msg);
            strncpy(response, me->msg, RKMaximumStringLength - 1);

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

RKPedestalVcpHandle *pedestalVcpInit(void) {
    RKPedestalVcpHandle *V = (RKPedestalVcpHandle *)malloc(sizeof(RKPedestalVcpHandle));
    if (V == NULL) {
        fprintf(stderr, "Unable to allocate resources for VCP.\n");
        return NULL;
    }
    memset(V, 0, sizeof(RKPedestalVcpHandle));
    snprintf(V->name, 64, "VCP");
    V->option = RKVcpOptionRepeat;
    V->active = false;
    return V;
}

RKPedestalAction pedestalVcpGetAction(RKPedestalPedzy *me) {
    float umin_diff_el;
    float umin_diff_az;
    float umin_diff_vel_el;
    float umin_diff_vel_az;
   
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;
    RKPedestalVcpHandle *V = me->vcpHandle;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = V->sweepMarkerElevation;
    action.sweepAzimuth = V->sweepAzimuth;
    if (V->sweepCount == 0 || V->active == false) {
        return action;
    }

    float g;
    float target_diff_az;
    float target_diff_el;
    float marker_diff_az;

    target_diff_az = pos->azimuthDegrees - V->batterSweeps[V->i].azimuthEnd;
    if (target_diff_az < 0.0f) {
        target_diff_az += 360.0f;
    }

    marker_diff_az = pos->azimuthDegrees - V->batterSweeps[V->i].azimuthMark;
    if (marker_diff_az < 0.0f) {
        marker_diff_az += 360.0f;
    }

    if (V->progress == RKVcpProgressNone) {
        // Clear the sweep start mark
        V->progress &= ~RKVcpProgressMarker;
        // Reposition the axes
        switch (V->batterSweeps[V->i].mode) {
            case RKVcpModeSector:
            case RKVcpModeNewSector:
            case RKVcpModeSpeedDown:
            case RKVcpModeRHI:
                break;
            case RKVcpModePPI:
            case RKVcpModePPIAzimuthStep:
            case RKVcpModePPIContinuous:
                umin_diff_el = umin_diff(V->batterSweeps[V->i].elevationStart, pos->elevationDegrees);
                if (umin_diff_el > RKPedestalPositionRoom){
                    pedestalElevationPoint(me, V->batterSweeps[V->i].elevationStart, V->batterSweeps[V->i].azimuthSlew);
                    action = pedestalElevationPointNudge(me, V->batterSweeps[V->i].elevationStart, V->batterSweeps[V->i].azimuthSlew);
                }else{
                    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action.param[0] = 0.0f;
                    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action.param[1] = V->batterSweeps[V->i].azimuthSlew;
                }
                break;
            default:
                break;
        }
        // Setup the start
        V->progress = RKVcpProgressSetup;
        V->tic = 0;

    } else if (V->progress == RKVcpProgressSetup) {
        // Compute the position difference from the target
        umin_diff_el = umin_diff(V->batterSweeps[V->i].elevationStart, pos->elevationDegrees);
        umin_diff_az = umin_diff(V->batterSweeps[V->i].azimuthStart, pos->azimuthDegrees);
        // Declare ready when certain conditions are met
        switch (V->batterSweeps[V->i].mode) {
            case RKVcpModeNewSector:
            case RKVcpModeSector:
            case RKVcpModeRHI:
            case RKVcpModeSpeedDown:
                switch (V->batterSweeps[V->i].mode) {
                    case RKVcpModeSector:
                        V->sweepAzimuth = 0.0f;
                        V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                        V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                        printf("Ready for EL %.2f\n", V->sweepElevation);
                        break;
                    case RKVcpModeRHI:
                        V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                        V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                        V->sweepMarkerElevation = V->batterSweeps[V->i].elevationEnd;
                        printf("Ready for AZ %.2f\n", V->sweepAzimuth);
                        break;
                    default:
                        V->sweepAzimuth = 0.0f;
                        V->sweepElevation = 0.0f;
                        V->sweepMarkerElevation = 0.0f;
                        break;
                }
                V->progress = RKVcpProgressReady;
                break;
            case RKVcpModePPI:
            case RKVcpModePPIAzimuthStep:
            case RKVcpModePPIContinuous:
                // Only wait for the EL axis
                if (umin_diff_el < RKPedestalPositionRoom &&
                    pos->elevationVelocityDegreesPerSecond > -RKPedestalVelocityRoom &&
                    pos->elevationVelocityDegreesPerSecond < RKPedestalVelocityRoom) {
                    V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                    V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                    V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                    printf("\033[1;32mFirst start for sweep %d - EL %.2f  @ crossover AZ %.2f   umin_diff_az=%.2f\033[0m\n", V->i, V->sweepElevation, V->sweepAzimuth, umin_diff_az);
                    V->progress = RKVcpProgressReady;
                }
                break;
            default:
                break;
        }
        if (V->tic > RKPedestalPointTimeOut) {
            printf("Too long to stabilize  tic = %d / %d   umin_diff_el=%.2f  umin_diff_az=%.2f  vel=%.2f  vaz=%.2f\n",
                   V->tic, RKPedestalPointTimeOut, umin_diff_el, umin_diff_az, pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
            // Re-arm the sweep
            V->progress = RKVcpProgressNone;
            V->tic = 0;
        }

    } else if (V->progress & RKVcpProgressReady) {

        // Get the next sweep, set the goals
        switch (V->batterSweeps[V->i].mode) {
            case RKVcpModeSector:
            case RKVcpModePPI:
                V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                // V->sweep_az = V->sweeps[V->i].az_start;
                V->targetAzimuth = V->batterSweeps[V->i].azimuthEnd - V->batterSweeps[V->i].azimuthStart;
                if (V->targetAzimuth <= 0.0f && V->batterSweeps[V->i].azimuthSlew > 0.0f) {
                    V->targetAzimuth += 361.0f;
                } else if (V->targetAzimuth >= 0.0f && V->batterSweeps[V->i].azimuthSlew < 0.0f) {
                    V->targetAzimuth -= 361.0f;
                }
                //printf("target_az = %.2f\n", V->target_az);
                V->counterTargetAzimuth = 0;
                V->targetElevation = V->batterSweeps[V->i].elevationStart;
                action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                action.param[1] = V->batterSweeps[V->i].azimuthSlew;
                umin_diff_el = umin_diff(V->targetElevation, pos->elevationDegrees);
                if (umin_diff_el < RKPedestalPositionRoom) {
                    printf("Lock elevation axis.  param[1] = %.2f\n", action.param[1]);
                    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    // Mark the position as active (ready for radar to start counting sweep)
                    //pos->flag |= POSITION_FLAG_SCAN_ACTIVE;
                    V->counterTargetAzimuth = 0;
                } else {
                    pedestalElevationPoint(me, V->targetElevation, V->batterSweeps[V->i].azimuthSlew);
                    action = pedestalElevationPointNudge(me, V->targetElevation, V->batterSweeps[V->i].azimuthSlew);
                }
                break;
            case RKVcpModeRHI:
                V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterSweeps[V->i].elevationEnd;
                printf("Ready for AZ %.2f\n", V->sweepAzimuth);

                umin_diff_el = umin_diff(V->batterSweeps[V->i].elevationStart, pos->elevationDegrees);
                umin_diff_az = umin_diff(V->batterSweeps[V->i].azimuthStart, pos->azimuthDegrees);
                if (umin_diff_el > RKPedestalPositionRoom || umin_diff_az > RKPedestalPositionRoom){
                    pedestalPoint(me, V->batterSweeps[V->i].elevationStart, V->batterSweeps[V->i].azimuthStart);
                }
                // action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
                // action.param[0] = 0.0f;
                // action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
                // action.param[1] = 0.0f;

                V->targetElevation = V->batterSweeps[V->i].elevationEnd - V->batterSweeps[V->i].elevationStart;
                V->targetAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->counterTargetElevation = 0;
                action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                action.param[0] = V->batterSweeps[V->i].elevationSlew;
                umin_diff_az = umin_diff(pos->azimuthDegrees, V->targetAzimuth);
                if (umin_diff_az < RKPedestalPositionRoom) {
                    action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    //pos->flag |= POSITION_FLAG_SCAN_ACTIVE;
                    V->counterTargetAzimuth = 0;
                } else {
                    pedestalAzimuthPoint(me, V->targetAzimuth, V->batterSweeps[V->i].elevationSlew);
                    action = pedestalAzimuthPointNudge(me, V->targetAzimuth, V->batterSweeps[V->i].elevationSlew);
                }
                break;
            case RKVcpModePPIAzimuthStep:
                V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                //pos->flag |= POSITION_FLAG_SCAN_ACTIVE;
                printf("\033[1;33mReady for sweep %d - EL %.2f  @ crossover AZ %.2f\033[0m\n", V->i, action.sweepElevation, action.sweepAzimuth);
                umin_diff_el = umin_diff(V->sweepElevation, pos->elevationDegrees);
                if (umin_diff_el > RKPedestalPositionRoom) {
                    pedestalElevationPoint(me, V->sweepElevation, V->batterSweeps[V->i].azimuthSlew);
                    action = pedestalElevationPointNudge(me, V->sweepElevation, V->batterSweeps[V->i].azimuthSlew);
                } else if ((pos->elevationVelocityDegreesPerSecond > RKPedestalVelocityRoom ||
                            pos->elevationVelocityDegreesPerSecond < -RKPedestalVelocityRoom)) {
                    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                    action.param[0] = 0.0f;
                    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action.param[1] = V->batterSweeps[V->i].azimuthSlew;
                }
                break;
            case RKVcpModePPIContinuous:
                V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                umin_diff_el = umin_diff(V->sweepElevation, pos->elevationDegrees);
                if (umin_diff_el < RKPedestalPositionRoom) {
                    //pos->flag |= POSITION_FLAG_SCAN_ACTIVE;
                    if ((pos->elevationVelocityDegreesPerSecond > RKPedestalVelocityRoom 
                            || pos->elevationVelocityDegreesPerSecond < -RKPedestalVelocityRoom)) {
                        action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                        action.param[0] = 0.0f;
                        action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                        action.param[1] = V->batterSweeps[V->i].azimuthSlew;
                    }
                    //pos->flag |= POSITION_FLAG_SCAN_ACTIVE;
                    V->counterTargetAzimuth = 0;
                }
                break;
            case RKVcpModeNewSector:
                V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                //pos->flag |= POSITION_FLAG_SCAN_ACTIVE;
                V->targetAzimuth = V->batterSweeps[V->i].azimuthEnd - pos->azimuthDegrees;
                if (V->batterSweeps[V->i].azimuthSlew > 0.0 && V->targetAzimuth < 0.0) {
                    V->targetAzimuth += 360.0;
                } else if (V->batterSweeps[V->i].azimuthSlew < 0.0 && V->targetAzimuth > 0.0) {
                    V->targetAzimuth -= 360.0;
                }
                printf("VCP_PROGRESS_READY: target_az_count = %.2f  (tgt:%.2f - cur:%.2f)\n", V->targetAzimuth, V->batterSweeps[V->i].azimuthEnd, pos->azimuthDegrees);
                action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeSlew;
                action.param[1] = V->batterSweeps[V->i].azimuthSlew;

                //printf("target_az = %.2f\n", V->target_az);
                V->counterTargetAzimuth = 0;
                V->targetElevation = V->batterSweeps[V->i].elevationStart;
                break;
            case RKVcpModeSpeedDown:
                V->sweepAzimuth = 0.0f;
                V->sweepElevation = 0.0f;
                V->sweepMarkerElevation = 0.0f;
                umin_diff_vel_el = umin_diff(0.0f, pos->elevationVelocityDegreesPerSecond);
                umin_diff_vel_az = umin_diff(0.0f, pos->azimuthVelocityDegreesPerSecond);
                int tic = 0;
                while ((umin_diff_vel_el > RKPedestalVelocityRoom || umin_diff_vel_az > RKPedestalVelocityRoom)
                    && tic < RKPedestalPointTimeOut) {
                    pos = RKGetLatestPosition(me->radar);
                    umin_diff_vel_el = umin_diff(0.0f, pos->elevationVelocityDegreesPerSecond);
                    umin_diff_vel_az = umin_diff(0.0f, pos->azimuthVelocityDegreesPerSecond);
                    action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
                    action.param[0] = pos->elevationVelocityDegreesPerSecond * 0.8;
                    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
                    action.param[1] = pos->azimuthVelocityDegreesPerSecond * 0.8;
                    printf("EL slew %.2f | AZ slew %.2f dps\n",
                        pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
                    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
                    tic++;
                    usleep(20000);
                }
                action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
                action.param[0] = 0;
                action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                action.param[1] = 0;
                printf("%s","Pedestal break finish.");
                V->progress = RKVcpProgressEnd;
                break;
            default:
                break;
        }
        // This stage the sweep is just about to start
        V->progress = RKVcpProgressMiddle;
        V->tic = 0;

    } else if (V->progress & RKVcpProgressMiddle) {

        // Middle of a sweep, check for sweep completeness
        switch (V->batterSweeps[V->i].mode) {
            case RKVcpModeSector:
            case RKVcpModePPI:
                V->counterTargetAzimuth += min_diff(pos->azimuthDegrees, V->azimuthPrevious);
                //printf("V->counter_az = %.2f <- %.2f\n", V->counter_az, diff_az);
                if ((V->batterSweeps[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                    (V->batterSweeps[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                    action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
                    V->progress = RKVcpProgressEnd | RKVcpProgressMarker;
                    // Mark the position as inactive (for radar to stop counting sweep)
                    //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
                }
                // If the elevation axis is still moving and the previous command has been a while
                if ((V->tic > 100 && (pos->elevationVelocityDegreesPerSecond < -0.05f ||
                    pos->elevationVelocityDegreesPerSecond > 0.05f))) {
                    target_diff_el = umin_diff(pos->elevationDegrees, V->targetElevation);
                    if (target_diff_el < RKPedestalPositionRoom) {
                        int k = action.mode[0] == RKPedestalInstructTypeNone ? 0 : 1;
                        action.mode[k] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
                        printf("Lock elevation axis again.  k = %d\n", k);
                        V->tic = 0;
                    }
                }
                break;
            case RKVcpModeRHI:
                V->counterTargetElevation += min_diff(pos->elevationDegrees, V->elevationPrevious);
                // if (V->progress & RKVcpProgressMarker) {
                //     V->progress ^= RKVcpProgressMarker;
                // } else if ((V->batterSweeps[V->i].elevationSlew > 0.0f && V->counterTargetElevation >= V->targetElevation) ||
                if ((V->batterSweeps[V->i].elevationSlew > 0.0f && V->counterTargetElevation >= V->targetElevation) ||
                    (V->batterSweeps[V->i].elevationSlew < 0.0f && V->counterTargetElevation <= V->targetElevation)) {
                    action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
                    V->progress = RKVcpProgressEnd | RKVcpProgressMarker;
                    // Mark the position as inactive (for radar to stop counting sweep)
                    //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
                }
                // If the azimuth axis is still moving and the previous command has been a while
                if (V->tic > 100 && (pos->azimuthVelocityDegreesPerSecond < -RKPedestalVelocityRoom ||
                pos->azimuthVelocityDegreesPerSecond > RKPedestalVelocityRoom)) {
                    target_diff_az = umin_diff(pos->azimuthDegrees, V->targetAzimuth);
                    if (target_diff_az < RKPedestalPositionRoom) {
                        int k = action.mode[0] == RKPedestalInstructTypeNone ? 0 : 1;
                        action.mode[k] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
                        printf("Lock azimuth axis again.  vaz = %.2f\n", pos->azimuthVelocityDegreesPerSecond);
                        V->tic = 0;
                    }
                }
                break;
            case RKVcpModePPIAzimuthStep:
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
                    if (V->option & RKVcpOptionVerbose) {
                        printf("Target cross over for %.2f detected @ %.2f.  %.2f %.2f\n", V->batterSweeps[V->i].azimuthEnd, pos->azimuthDegrees, V->targetDiffAzimuthPrevious, target_diff_az);
                    }
                    V->progress |= RKVcpProgressEnd;
                    V->counterTargetAzimuth = 0;
                }
                g = marker_diff_az - V->markerDiffAzimuthPrevious;
                // if (V->progress & RKVcpProgressMarker) {
                //     V->progress ^= RKVcpProgressMarker;
                // } else if (V->counterMarkerAzimuth > 180.0f && (g < -350.0f || g > 350.0f)) {
                if (V->counterMarkerAzimuth > 180.0f && (g < -350.0f || g > 350.0f)) {
                    if (V->option & RKVcpOptionVerbose) {
                        printf("Marker cross over for %.2f detected @ %.2f.  %.2f %.2f\n", V->batterSweeps[V->i].azimuthMark, pos->azimuthDegrees, V->markerDiffAzimuthPrevious, marker_diff_az);
                    }
                    V->progress |= RKVcpProgressMarker;
                    V->counterMarkerAzimuth = 0;
                    // Sweep marked, go to the next sweep
                    V->j = (V->j == V->sweepCount - 1) ? 0 : V->j + 1;
                    // V->sweepMarkerElevation = V->batterSweeps[V->j].elevationStart;
                }
                g = umin_diff(pos->azimuthDegrees, V->azimuthPrevious);
                V->counterTargetAzimuth += g;
                V->counterMarkerAzimuth += g;
                break;
            case RKVcpModePPIContinuous:
                V->counterTargetAzimuth += min_diff(pos->azimuthDegrees, V->azimuthPrevious);
                if ((V->batterSweeps[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                    (V->batterSweeps[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                    V->progress |= RKVcpProgressEnd;
                    V->counterTargetAzimuth = 0;
                }
                break;
            case RKVcpModeNewSector:
                // V->counterTargetAzimuth += min_diff(pos->azimuthDegrees, V->azimuthPrevious);
                // //printf("V->counter_az = %.2f <- %.2f\n", V->counter_az, diff_az);
                // if ((V->batterSweeps[V->i].azimuthSlew > 0.0f && V->counterTargetAzimuth >= V->targetAzimuth) ||
                //     (V->batterSweeps[V->i].azimuthSlew < 0.0f && V->counterTargetAzimuth <= V->targetAzimuth)) {
                //     action.mode[0] = INSTRUCT_MODE_STANDBY | INSTRUCT_AXIS_AZ;
                //     // Look ahead of to get ready for the next elevation
                //     int k = V->i == V->sweep_count - 1 ? 0 : V->i + 1;
                //     target_diff_el = umin_diff(pos->el_deg, V->sweeps[k].el_start);
                //     if (target_diff_el < VCP_POS_TOL) {
                //         action.mode[1] = INSTRUCT_MODE_POINT | INSTRUCT_AXIS_EL;
                //         action.param[1] = V->sweeps[k].el_start;
                //     } else {
                //         action.mode[1] = INSTRUCT_MODE_RESET | INSTRUCT_AXIS_EL;
                //     }
                //     V->progress = VCP_PROGRESS_ENDED | VCP_PROGRESS_MARKER;
                //     // Mark the position as inactive (for radar to stop counting sweep)
                //     //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
                // } else if (V->option & VCP_OPTION_BRAKE_EL_DURING_SWEEP &&
                //     // If the elevation axis is still moving and the previous command has been a while
                //     (V->tic > 100 && (pos->vel_dps < -0.05f || pos->vel_dps > 0.05f))) {
                //     target_diff_el = umin_diff(pos->el_deg, V->target_el);
                //     if (target_diff_el < VCP_POS_TOL) {
                //         //int k = action.mode[0] == INSTRUCT_NONE ? 0 : 1;
                //         //action.mode[k] = INSTRUCT_AXIS_EL | INSTRUCT_MODE_STANDBY;
                //         action.mode[0] = INSTRUCT_AXIS_EL | INSTRUCT_MODE_STANDBY;
                //         printf("Lock elevation axis again.\n");
                //         V->tic = 0;
                //     }
                // }
                break;
            case RKVcpModeSpeedDown:
                V->progress = RKVcpProgressEnd;
                break;
            default:
                break;
        } // switch (V->sweeps[V->i].mode) ...
    }

    if (V->progress & RKVcpProgressEnd) {

        // Ended the sweep, check for completeness
        switch (V->batterSweeps[V->i].mode) {
            case RKVcpModeSpeedDown:
            case RKVcpModeSector:
            case RKVcpModePPI:
            case RKVcpModeRHI:
                // Sweep ended, wait until the pedestal stops, then go to the next sweep
                if (pos->azimuthVelocityDegreesPerSecond > -RKPedestalVelocityRoom &&
                    pos->azimuthVelocityDegreesPerSecond < RKPedestalVelocityRoom) {
                    V->i = (V->i == V->sweepCount - 1) ? 0 : V->i + 1;
                    if (V->i == 0) {
                        if (V->option & RKVcpOptionRepeat) {
                            pedestalVcpNextHitter(me->vcpHandle);
                            printf("VCP repeats.\n");
                        } else {
                            printf("VCP stops.\n");
                            V->active = false;
                            //action.mode[0] = INSTRUCT_AXIS_EL | INSTRUCT_STANDBY;
                            //action.mode[1] = INSTRUCT_AXIS_AZ | INSTRUCT_STANDBY;
                            //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
                        }
                    }
                    V->progress |= RKVcpProgressReady;
                    V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                    V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                    V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                }
                break;
            case RKVcpModePPIAzimuthStep:
                // Sweep ended, go to the next sweep
                V->i = (V->i == V->sweepCount - 1) ? 0 : V->i + 1;
                if (V->i == 0) {
                    if (V->option & RKVcpOptionRepeat) {
                        pedestalVcpNextHitter(me->vcpHandle);
                        printf("VCP repeats.\n");
                        // V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                        // V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                        V->progress |= RKVcpProgressReady;
                    } else {
                        printf("VCP stops.\n");
                        V->active = false;
                        V->progress &= ~RKVcpProgressMiddle;
                        //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
                        break;
                    }
                } else {
                    // V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                    // V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                    V->progress |= RKVcpProgressReady;
                }
                V->sweepAzimuth = V->batterSweeps[V->i].azimuthStart;
                V->sweepElevation = V->batterSweeps[V->i].elevationStart;
                V->sweepMarkerElevation = V->batterSweeps[V->i].elevationStart;
                // if (V->batterSweeps[V->i].mode != RKVcpModePPIAzimuthStep) {
                //     V->progress |= RKVcpOptionNone;
                // }
                break;
            case RKVcpModePPIContinuous:
                break;
            // case VCP_MODE_NEW_SEC:
            //     //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
            //     if (V->progress & VCP_PROGRESS_MARKER) {
            //         V->progress ^= VCP_PROGRESS_MARKER;
            //     }
            //     if (pos->vaz_dps > -VCP_VEL_TOL && pos->vaz_dps < VCP_VEL_TOL &&
            //         pos->vel_dps > -VCP_VEL_TOL && pos->vel_dps < VCP_VEL_TOL) {
            //         V->i = (V->i == V->sweep_count - 1) ? 0 : V->i + 1;
            //         if (V->i == 0 && !(V->option & VCP_OPTION_REPEAT)) {
            //             printf("VCP stops.\n");
            //             V->active = false;
            //             //action.mode[0] = INSTRUCT_AXIS_EL | INSTRUCT_STANDBY;
            //             //action.mode[1] = INSTRUCT_AXIS_AZ | INSTRUCT_STANDBY;
            //             //pos->flag &= ~POSITION_FLAG_SCAN_ACTIVE;
            //         } else {
            //             printf("VCP_PROGRESS_ENDED   Next i = %d   pos->vaz_dps = %.2f dps\n", V->i, pos->vaz_dps);
            //             V->sweep_az = 0.0f;
            //             V->sweep_el = V->sweeps[V->i].el_start;
            //             V->sweep_marker_el = V->sweeps[V->i].el_start;
            //             V->progress = VCP_PROGRESS_SETUP;
            //         }
            //     }
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

    action.sweepElevation = V->sweepMarkerElevation;
    action.sweepAzimuth = V->sweepAzimuth;

    return action;
}

void pedestalVcpArmSweeps(RKPedestalVcpHandle *V, const bool repeat) {
    V->progress = RKVcpProgressNone;
    if (repeat) {
        V->option |= RKVcpOptionNone;
    }
    V->active = true;
    V->i = 0;
    V->j = 0;
}

void pedestalVcpClearSweeps(RKPedestalVcpHandle *V) {
    V->progress = RKVcpProgressNone;
    V->sweepCount = 0;
    V->onDeckCount = 0;
    V->inTheHoleCount = 0;
    V->i = 0;
    V->j = 0;
    V->active = true;
}

void pedestalVcpClearHole(RKPedestalVcpHandle *V) {
    V->inTheHoleCount = 0;
    V->onDeckCount = 0;
}

void pedestalVcpClearDeck(RKPedestalVcpHandle *V) {
    V->onDeckCount = 0;
}

void pedestalVcpNextHitter(RKPedestalVcpHandle *V) {
    memset(V->batterSweeps, 0, sizeof(RKPedestalVcpSweepHandle));
    memcpy(V->batterSweeps, V->onDeckSweeps, V->onDeckCount * sizeof(RKPedestalVcpSweepHandle));
    memset(V->onDeckSweeps, 0, sizeof(RKPedestalVcpSweepHandle));
    memcpy(V->onDeckSweeps, V->inTheHoleSweeps, (V->onDeckCount+V->inTheHoleCount) * sizeof(RKPedestalVcpSweepHandle));
    V->sweepCount = V->onDeckCount;
    V->onDeckCount = V->inTheHoleCount;
    V->i = 0;
    V->j = 0;
    V->active = true;
}

RKPedestalVcpSweepHandle pedestalVcpMakeSweep(RKVcpMode mode,
                       const float el_start, const float el_end,
                       const float az_start, const float az_end, const float az_mark,
                       const float rate) {
    RKPedestalVcpSweepHandle sweep;
    memset(&sweep, 0, sizeof(RKPedestalVcpSweepHandle));
    sweep.mode     = mode;
    sweep.azimuthStart = az_start;
    sweep.azimuthEnd   = az_end;
    sweep.azimuthMark  = az_mark;
    sweep.azimuthSlew  = rate;
    sweep.elevationStart = el_start;
    sweep.elevationEnd   = el_end;
    sweep.elevationSlew  = rate;
    return sweep;
}

int pedestalVcpAddLineupSweep(RKPedestalVcpHandle *V, RKPedestalVcpSweepHandle sweep) {
    if (V->inTheHoleCount < RKPedestalVcpMaxSweeps - 1) {
        V->inTheHoleSweeps[V->inTheHoleCount++] = sweep;
        V->onDeckSweeps[V->onDeckCount++] = sweep;
    } else {
        fprintf(stderr, "Cannot add more tilts. Currently %d.\n", V->inTheHoleCount);
        return 1;
    }
    return 0;
}

int pedestalVcpAddPinchSweep(RKPedestalVcpHandle *V, RKPedestalVcpSweepHandle sweep) {
    if (V->onDeckCount < RKPedestalVcpMaxSweeps - 1) {
        V->onDeckSweeps[V->onDeckCount++] = sweep;
    } else {
        fprintf(stderr, "Cannot add more tilts. Currently %d.\n", V->onDeckCount);
        return 1;
    }
    return 0;
}

void pedestalVcpSendAction(int sd, char *ship, RKPedestalAction *act) {
    sprintf(ship, "%c", 0x0f);
    memcpy(ship + 1, act, sizeof(RKPedestalAction));
    strncpy(ship + 1 + sizeof(RKPedestalAction), RKEOL, 2);
    // printf("EL %.2f dps| AZ %.2f dps \n",act->param[0],act->param[1]);
    RKNetworkSendPackets(sd, ship, sizeof(RKPedestalAction) + 3, NULL);
}

void makeSweepMessage(RKPedestalVcpSweepHandle *SW, char *msg, int SC, RKVcpHitter linetag) {
    char prefix[7];
    switch (linetag){
        case RKVcpAtBat:
            strncpy(prefix, "       ", 7);
            break;
        case RKVcpPinch:
            strncpy(prefix, "PINCH  ", 7);
            break;
        case RKVcpLine:
            strncpy(prefix, "LINEUP ", 7);
            break;
        default:
            break;
    }

    for (int i=0; i<SC; i++) {
        switch (SW[i].mode) {
            case RKVcpModePPI:
                sprintf(msg + strlen(msg), "%s%d : PPI E%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        SW[i].elevationStart,
                        SW[i].azimuthSlew);
                break;
            case RKVcpModeSector:
            case RKVcpModeNewSector:
                sprintf(msg + strlen(msg), "%s%d : SEC E%.2f A%.2f-%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        SW[i].elevationStart,
                        SW[i].azimuthStart,
                        SW[i].azimuthEnd,
                        SW[i].azimuthSlew);
                break;
            case RKVcpModeRHI:
                sprintf(msg + strlen(msg), "%s%d : RHI A%.2f E%.2f-%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        SW[i].azimuthStart,
                        SW[i].elevationStart,
                        SW[i].elevationEnd,
                        SW[i].elevationSlew);
                break;
            case RKVcpModePPIAzimuthStep:
            case RKVcpModePPIContinuous:
                sprintf(msg + strlen(msg), "%s%d : PPI_NEW E%.2f A%.2f @ %.2f deg/s",
                        prefix,
                        i,
                        SW[i].elevationStart,
                        SW[i].azimuthMark,
                        SW[i].azimuthSlew);
                break;
            case RKVcpModeSpeedDown:
                sprintf(msg + strlen(msg), "%s%d : PPI_SLOWDOWN",
                        prefix,
                        i);
            default:
                break;
        }
        sprintf(msg + strlen(msg), "\n");
    }
}

void pedestalVcpSummary(RKPedestalVcpHandle *V, char *msg) {
    msg[0] = '\0';
    makeSweepMessage(V->batterSweeps, msg, V->sweepCount, RKVcpAtBat);
    if (memcmp(V->inTheHoleSweeps, V->onDeckSweeps, (V->onDeckCount+V->inTheHoleCount)*sizeof(RKPedestalVcpSweepHandle))) {
        makeSweepMessage(V->onDeckSweeps, msg, V->onDeckCount, RKVcpPinch);
    }
    makeSweepMessage(V->inTheHoleSweeps, msg, V->inTheHoleCount, RKVcpLine);
    printf("================================================\n"
           "VCP Summary:\n"
           "------------\n"
           "%s"
           "================================================\n", msg);
}

float pedestalGetRate(const float diff_deg, int axis){
    float rate = 0.0f;
    if ( axis == RKPedestalPointAzimuth ){
        if ( diff_deg >= 20.0f){
            rate = 30.0f;
        } else if ( diff_deg >= 10.0f){
            rate = 20.0f;
        } else if ( diff_deg >= 5.0f){
            rate = 10.0f;
        } else if ( diff_deg < 5.0f){
            rate = 3.0f;
        }
    } else if ( axis == RKPedestalPointElevation ){
        if ( diff_deg >= 20.0f){
            rate = 15.0f;
        } else if ( diff_deg >= 7.0f){
            rate = 10.0f;
        } else if ( diff_deg < 7.0f){
            rate = 3.0f;
        }
    }
    return rate;
}

int pedestalPoint(RKPedestalPedzy *me, const float el_point, const float az_point){
    float umin_diff_el;
    float umin_diff_az;
    float min_diff_el;
    float min_diff_az;
    float rate_el;
    float rate_az;
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = el_point;
    action.sweepAzimuth = az_point;

    umin_diff_el = umin_diff(el_point, pos->elevationDegrees);
    umin_diff_az = umin_diff(az_point, pos->azimuthDegrees);
    int i = 0;
    while ((umin_diff_el > RKPedestalPositionRoom || umin_diff_az > RKPedestalPositionRoom)
        && i < RKPedestalPointTimeOut) {

        pos = RKGetLatestPosition(me->radar);
        umin_diff_el = umin_diff(el_point, pos->elevationDegrees);
        if (umin_diff_el > RKPedestalPositionRoom){
            action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
            rate_el = pedestalGetRate( umin_diff_el, RKPedestalPointElevation );
            min_diff_el = min_diff(el_point, pos->elevationDegrees);
            if (min_diff_el >= 0.0f){
                action.param[0] = rate_el;
            }else{
                action.param[0] = -rate_el;
            }
        } else{
            action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
            action.param[0] = 0.0f;
        }

        umin_diff_az = umin_diff(az_point, pos->azimuthDegrees);
        if (umin_diff_az > RKPedestalPositionRoom){
            action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
            rate_az = pedestalGetRate( umin_diff_az, RKPedestalPointAzimuth );
            min_diff_az = min_diff(az_point, pos->azimuthDegrees);
            if (min_diff_az >= 0.0f){
                action.param[1] = rate_az;
            }else{
                action.param[1] = -rate_az;
            }
        } else{
            action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
            action.param[1] = 0.0f;
        }
        // printf("EL point %.2f | AZ point %.2f \n",pos->elevationDegrees,pos->azimuthDegrees);
        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
        i++;
        usleep(20000);
    }
    action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
    action.param[0] = 0.0f;
    action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
    action.param[1] = 0.0f;
    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
    printf("%s","Point finish.");
    if (i < RKPedestalPointTimeOut){
        return 0;
    }else{
        return 1;
    }
}

int pedestalAzimuthPoint(RKPedestalPedzy *me, const float az_point, const float rate_el){
    float umin_diff_az;
    float min_diff_az;
    float rate_az;
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = pos->elevationDegrees;
    action.sweepAzimuth = az_point;
    umin_diff_az = umin_diff(az_point, pos->azimuthDegrees);
    int i = 0;
    while (umin_diff_az > RKPedestalPositionRoom && i < RKPedestalPointTimeOut) {
        pos = RKGetLatestPosition(me->radar);
        umin_diff_az = umin_diff(az_point, pos->azimuthDegrees);
        if (umin_diff_az > RKPedestalPositionRoom){
            action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
            rate_az = pedestalGetRate( umin_diff_az, RKPedestalPointAzimuth );
            min_diff_az = min_diff(az_point, pos->azimuthDegrees);
            if (min_diff_az >= 0.0f){
                action.param[1] = rate_az;
            }else{
                action.param[1] = -rate_az;
            }
        } else{
            action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
            action.param[1] = 0.0f;
        }
        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
        action.param[0] = rate_el;
        // printf("EL slew %.2f | AZ point %.2f dps\n",rate_el,pos->azimuthDegrees);
        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
        i++;
        usleep(20000);
    }
    action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
    action.param[0] = rate_el;
    action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
    action.param[1] = 0.0f;
    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
    printf("%s","Azimuth point finish.");
    if (i < RKPedestalPointTimeOut){
        return 0;
    }else{
        return 1;
    }
}

int pedestalElevationPoint(RKPedestalPedzy *me, const float el_point, const float rate_az){
    float umin_diff_el;
    float min_diff_el;
    float rate_el;
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = el_point;
    action.sweepAzimuth = pos->azimuthDegrees;
    umin_diff_el = umin_diff(el_point, pos->elevationDegrees);
    int i = 0;
    while (umin_diff_el > RKPedestalPositionRoom && i < RKPedestalPointTimeOut) {

        pos = RKGetLatestPosition(me->radar);
        umin_diff_el = umin_diff(el_point, pos->elevationDegrees);
        if (umin_diff_el > RKPedestalPositionRoom){
            action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
            rate_el = pedestalGetRate( umin_diff_el, RKPedestalPointElevation );
            min_diff_el = min_diff(el_point, pos->elevationDegrees);
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
        // printf("EL point %.2f | AZ slew %.2f dps\n",pos->elevationDegrees,rate_az);
        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
        i++;
        usleep(20000);
    }
    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
    action.param[0] = 0.0f;
    action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
    action.param[1] = rate_az;
    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
    printf("%s","Elevation point finish.");
    if (i < RKPedestalPointTimeOut){
        return 0;
    }else{
        return 1;
    }
}


RKPedestalAction pedestalAzimuthPointNudge(RKPedestalPedzy *me, const float az_point, const float rate_el){
    float umin_diff_az;
    float min_diff_az;
    float rate_az;
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = pos->elevationDegrees;
    action.sweepAzimuth = az_point;
    umin_diff_az = umin_diff(az_point, pos->azimuthDegrees);
    if (umin_diff_az > RKPedestalPositionRoom){
        action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
        rate_az = pedestalGetRate( umin_diff_az, RKPedestalPointAzimuth );
        min_diff_az = min_diff(az_point, pos->azimuthDegrees);
        if (min_diff_az >= 0.0f){
            action.param[1] = rate_az;
        }else{
            action.param[1] = -rate_az;
        }
    } else{
        action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
        action.param[1] = 0.0f;
    }
    action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
    action.param[0] = rate_el;
    return action;
}


RKPedestalAction pedestalElevationPointNudge(RKPedestalPedzy *me, const float el_point, const float rate_az){
    float umin_diff_el;
    float min_diff_el;
    float rate_el;
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
    action.sweepElevation = el_point;
    action.sweepAzimuth = pos->azimuthDegrees;
    umin_diff_el = umin_diff(el_point, pos->elevationDegrees);
    if (umin_diff_el > RKPedestalPositionRoom){
        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
        rate_el = pedestalGetRate( umin_diff_el, RKPedestalPointElevation );
        min_diff_el = min_diff(el_point, pos->elevationDegrees);
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

int pedestalSlowDown(RKPedestalPedzy *me){
    float umin_diff_vel_el;
    float umin_diff_vel_az;
    RKPosition *pos = RKGetLatestPosition(me->radar);
    RKPedestalAction action;

    umin_diff_vel_el = umin_diff(0.0f, pos->elevationVelocityDegreesPerSecond);
    umin_diff_vel_az = umin_diff(0.0f, pos->azimuthVelocityDegreesPerSecond);
    int tic = 0;
    while ((umin_diff_vel_el > RKPedestalVelocityRoom || umin_diff_vel_az > RKPedestalVelocityRoom)
        && tic < RKPedestalPointTimeOut) {
        pos = RKGetLatestPosition(me->radar);
        umin_diff_vel_el = umin_diff(0.0f, pos->elevationVelocityDegreesPerSecond);
        umin_diff_vel_az = umin_diff(0.0f, pos->azimuthVelocityDegreesPerSecond);
        action.mode[0] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisElevation;
        action.param[0] = pos->elevationVelocityDegreesPerSecond * 0.8;
        action.mode[1] = RKPedestalInstructTypeModeSlew | RKPedestalInstructTypeAxisAzimuth;
        action.param[1] = pos->azimuthVelocityDegreesPerSecond * 0.8;
        printf("EL slew %.2f | AZ slew %.2f dps\n",
            pos->elevationVelocityDegreesPerSecond, pos->azimuthVelocityDegreesPerSecond);
        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
        tic++;
        usleep(20000);
    }
    action.mode[0] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisElevation;
    action.param[0] = 0;
    action.mode[1] = RKPedestalInstructTypeModeStandby | RKPedestalInstructTypeAxisAzimuth;
    action.param[1] = 0;
    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
    printf("%s","SlowDown finish.");
    if (tic < RKPedestalPointTimeOut){
        return 0;
    }else{
        return 1;
    }
}
