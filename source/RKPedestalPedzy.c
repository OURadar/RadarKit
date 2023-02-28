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

RKPedestalAction pedestalVcpGetAction(RKPedestalVcpHandle *, const RKPosition *);
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
void makeSweepMessage(RKPedestalVcpSweepHandle *, char *, int SC, bool linetag);
float pedestalGetRate(const float diff_deg, int axis);

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
        if (me->client->state < RKClientStateConnected) {
            azInterlockStatus = RKStatusEnumInvalid;
            elInterlockStatus = RKStatusEnumInvalid;
            vcpActive = RKStatusEnumInvalid;
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
            // sprintf(elPosition, "%.2f deg", position->elevationDegrees);
            // sprintf(azPosition, "%.2f deg", position->azimuthDegrees);
            azEnum = RKStatusEnumNormal;
            elEnum = RKStatusEnumNormal;

            if (me->vcpHandle->active){
                // put vcp get action here
                action = pedestalVcpGetAction(me->vcpHandle, position);
                // Call the control handler based on the suggested action
                if (action.mode[0] != RKPedestalInstructTypeNone || action.mode[1] != RKPedestalInstructTypeNone) {
                    // P->control(P->ped, action);
                    pedestalVcpSendAction(client->sd, me->latestCommand, &action);
                    // sprintf(me->latestCommand, "%c", 0x0f);
                    // memcpy(me->latestCommand + 1, &action, sizeof(RKPedestalAction));
                    // RKNetworkSendPackets(client->sd, me->latestCommand, sizeof(RKPedestalAction) + 1, NULL);
                    
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
        usleep(1000);
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
            immediatelyDo = false;
            onlyOnce = false;
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
                if (immediatelyDo) {
                    pedestalVcpNextHitter(me->vcpHandle);
                    pedestalVcpNextHitter(me->vcpHandle);
                }
                pedestalVcpSummary(me->vcpHandle, me->msg);
                printf("ACK. Volume added successfully." RKEOL);
            } else {
                printf("NAK. Some error occurred." RKEOL);
            }
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
    return V;
}

RKPedestalAction pedestalVcpGetAction(RKPedestalVcpHandle *V, const RKPosition *pos) {
    RKPedestalAction action;
    action.mode[0] = RKPedestalInstructTypeNone;
    action.mode[1] = RKPedestalInstructTypeNone;
    action.param[0] = 0.0f;
    action.param[1] = 0.0f;
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
    V->active = true;
    V->i = 0;
    V->j = 0;
}

void pedestalVcpClearHole(RKPedestalVcpHandle *V) {
    V->inTheHoleCount = 0;
}

void pedestalVcpClearDeck(RKPedestalVcpHandle *V) {
    V->onDeckCount = 0;
}

void pedestalVcpNextHitter(RKPedestalVcpHandle *V) {
    memset(V->batterSweeps, 0, sizeof(RKPedestalVcpSweepHandle));
    memcpy(V->batterSweeps, V->onDeckSweeps, sizeof(RKPedestalVcpSweepHandle));
    memset(V->onDeckSweeps, 0, sizeof(RKPedestalVcpSweepHandle));
    memcpy(V->onDeckSweeps, V->inTheHoleSweeps, sizeof(RKPedestalVcpSweepHandle));
    V->sweepCount = V->onDeckCount;
    V->onDeckCount = V->inTheHoleCount;
    V->active = true;
    V->i = 0;
    V->j = 0;
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
    RKNetworkSendPackets(sd, ship, sizeof(RKPedestalAction) + 1, NULL);
}

void makeSweepMessage(RKPedestalVcpSweepHandle *SW, char *msg, int SC, bool linetag) {
    char prefix[7];
    if (linetag) {
        strncpy(prefix, "LINEUP ", 7);
    } else {
        strncpy(prefix, "       ", 7);
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
    makeSweepMessage(V->batterSweeps, msg, V->sweepCount, false);
    if (!memcmp(V->inTheHoleSweeps, V->onDeckSweeps, sizeof(RKPedestalVcpSweepHandle))) {
        makeSweepMessage(V->onDeckSweeps, msg, V->onDeckCount, false);
    }
    makeSweepMessage(V->inTheHoleSweeps, msg, V->inTheHoleCount, true);
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
        } else if ( diff_deg >= 8.0f){
            rate = 10.0f;
        } else if ( diff_deg < 5.0f){
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
            min_diff_az = min_diff(el_point, pos->azimuthDegrees);
            if (min_diff_az >= 0.0f){
                action.param[1] = rate_az;
            }else{
                action.param[1] = -rate_az;
            }
        } else{
            action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
            action.param[1] = 0.0f;
        }

        pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
        i++;
        usleep(1000);
    }
    action.mode[0] = RKPedestalInstructTypeAxisElevation | RKPedestalInstructTypeModeStandby;
    action.param[0] = 0.0f;
    action.mode[1] = RKPedestalInstructTypeAxisAzimuth | RKPedestalInstructTypeModeStandby;
    action.param[1] = 0.0f;
    pedestalVcpSendAction(me->client->sd, me->latestCommand, &action);
}