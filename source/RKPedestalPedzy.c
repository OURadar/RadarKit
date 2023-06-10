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

static ssize_t pedestalPedzySendAction(int sd, char *ship, RKScanAction *act) {
    sprintf(ship, "%c", 0x0f);
    memcpy(ship + 1, act, sizeof(RKScanAction));
    strncpy(ship + 1 + sizeof(RKScanAction), RKEOL, 3);
    return RKNetworkSendPackets(sd, ship, sizeof(RKScanAction) + 3, NULL);
}

static int pedestalPedzyRead(RKClient *client) {
    // The shared user resource pointer
    RKPedestalPedzy *me = (RKPedestalPedzy *)client->userResource;

    RKRadar *radar = me->radar;

    RKSteerEngine *steerEngine = radar->steerEngine;

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
        newPosition->azimuthDegrees += radar->desc.heading;
        if (newPosition->azimuthDegrees < 0.0f) {
            newPosition->azimuthDegrees += 360.0f;
        } else if (newPosition->azimuthDegrees >= 360.0f) {
            newPosition->azimuthDegrees -= 360.0f;
        }
        newPosition->sweepAzimuthDegrees += radar->desc.heading;
        if (newPosition->sweepAzimuthDegrees < 0.0f) {
            newPosition->sweepAzimuthDegrees += 360.0f;
        } else if (newPosition->sweepAzimuthDegrees >= 360.0f) {
            newPosition->sweepAzimuthDegrees -= 360.0f;
        }
        // Correct overshooting elevation (only for overshooting greater than 180 which show -17X.X)
        // keep 0 ~ -1.X as it was
        if (newPosition->elevationDegrees < -90.0f) {
            newPosition->elevationDegrees += 360.0f;
        }
        // Some integrity check
        if (fabsf(newPosition->azimuthDegrees) > 360.0f || fabsf(newPosition->azimuthVelocityDegreesPerSecond) > 360.0f) {
            RKLog("%s Error. Unexpected azimuth reading %.1f° @ %.1f°/s\n", client->name,
                newPosition->azimuthDegrees, newPosition->azimuthVelocityDegreesPerSecond);
        }
        // Some more margin for overshooting
       if (fabsf(newPosition->elevationDegrees) > 200.0f || fabsf(newPosition->elevationVelocityDegreesPerSecond) > 100.0f) {
            RKLog("%s Error. Unexpected azimuth reading %.1f° @ %.1f°/s\n", client->name,
                newPosition->elevationDegrees, newPosition->elevationVelocityDegreesPerSecond);
        }

        // Note to myself: Could consider adding something similar to the RKRadar abstraction layer
        // That way, the thought process is consistent like RKGetPulse(), RKGetRay(), etc.

        RKScanAction *action = RKSteerEngineGetAction(steerEngine, newPosition);

        RKSetPositionReady(radar, newPosition);

        if (action->mode[0] & RKPedestalInstructTypeModeMask) {
            pedestalPedzySendAction(client->sd, me->latestCommand, action);
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

static int pedestalPedzyGreet(RKClient *client) {
    ssize_t s = RKNetworkSendPackets(client->sd, "\n", 1, NULL);
    if (s < 0) {
        return -s;
    }
    RKLog("%s Pedzy @ %s:%d connected.\n", client->name, client->hostIP, client->port);
    // The shared user resource pointer
    RKPedestalPedzy *me = (RKPedestalPedzy *)client->userResource;
    RKRadar *radar = me->radar;
    if (client->verbose > 1) {
        RKLog("%s Resetting position clock.\n", me->client->name);
    }
    RKClockReset(radar->positionClock);
    return RKResultSuccess;
}

static void *pedestalPedzyHealth(void *in) {
    RKPedestalPedzy *me = (RKPedestal)in;
    RKRadar *radar = me->radar;
    RKSteerEngine *steerer = radar->steerEngine;
    RKStatusEnum azInterlockStatus = RKStatusEnumInvalid;
    RKStatusEnum elInterlockStatus = RKStatusEnumInvalid;
    RKStatusEnum vcpStatusEnum;
    char azPosition[16];
    char elPosition[16];
    RKStatusEnum azEnum;
    RKStatusEnum elEnum;
    while (me->client->state < RKClientStateDisconnecting) {
        if (me->client->state < RKClientStateConnected) {
            azInterlockStatus = RKStatusEnumInvalid;
            elInterlockStatus = RKStatusEnumInvalid;
            vcpStatusEnum = RKStatusEnumInvalid;
            sprintf(elPosition, "--.-- deg");
            sprintf(azPosition, "--.-- deg");
            azEnum = RKStatusEnumInvalid;
            elEnum = RKStatusEnumInvalid;
        } else {
            RKPosition *position = RKGetLatestPosition(radar);
            azInterlockStatus = position->flag & RKPositionFlagAzimuthSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            elInterlockStatus = position->flag & RKPositionFlagElevationSafety ? RKStatusEnumNotOperational : RKStatusEnumNormal;
            if (position->flag & (RKPositionFlagAzimuthError | RKPositionFlagElevationError)) {
                vcpStatusEnum = RKStatusEnumFault;
            } else if (steerer->vcpHandle.active) {
                vcpStatusEnum = RKStatusEnumActive;
            } else {
                vcpStatusEnum = RKStatusEnumStandby;
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
                    vcpStatusEnum == RKStatusEnumActive ? "true" : "false", vcpStatusEnum,
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
        desc.port = atoi(colon + 1);
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
    RKClientSetGreetHandler(me->client, &pedestalPedzyGreet);
    RKClientSetReceiveHandler(me->client, &pedestalPedzyRead);
    RKClientStart(me->client, false);

    if (pthread_create(&me->tidBackground, NULL, pedestalPedzyHealth, me) != 0) {
        RKLog("%s Error. Failed to start a pedestal monitor.\n", me->client->name);
        return (void *)RKResultFailedToStartPedestalMonitor;
    }

    return (RKPedestal)me;
}

int RKPedestalPedzyExec(RKPedestal input, const char *command, char _Nullable *response) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClient *client = me->client;

    if (response == NULL) {
        response = (char *)me->response;
    }

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
            sprintf(response, "NAK. Pedestal not connected." RKEOL);
            return RKResultClientNotConnected;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumCommandLength, "%s" RKEOL, command);
        RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
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
