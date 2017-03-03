//
//  RKPedestalPedzy.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/4/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPedestalPedzy.h>

#pragma mark - Internal Functions

// Internal Implementations

int RKPedestalPedzyRead(RKClient *client) {
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
            RKLog("%s failed to get a vacant position.\n", client->name);
            return RKResultFailedToGetVacantPosition;
        }
        memcpy(newPosition, client->userPayload, sizeof(RKPosition));
        RKSetPositionReady(radar, newPosition);
    } else {
        // This the command acknowledgement, queue it up to feedback
        char *string = (char *)client->userPayload;
        if (!strncmp(string, "pong", 4)) {
            // Just a beacon response.
        } else {
            if (client->verbose && me->latestCommand[0] != 'h') {
                RKLog("%s %s", client->name, string);
            }
            strncpy(me->responses[me->responseIndex], client->userPayload, RKMaximumStringLength - 1);
            me->responseIndex = RKNextModuloS(me->responseIndex, RKPedestalPedzyFeedbackDepth);
        }
    }

    return RKResultSuccess;
}

#pragma mark - Protocol Implementations

// Implementations

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
    sprintf(desc.name, "%s<PedzyRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    strncpy(desc.hostname, (char *)input, RKMaximumStringLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9554;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatHeaderDefinedSize;
    desc.blocking = true;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose =
    radar->desc.initFlags & RKInitFlagVeryVeryVerbose ? 3 :
    (radar->desc.initFlags & RKInitFlagVeryVerbose ? 2:
     (radar->desc.initFlags & RKInitFlagVerbose ? 1 : 0));

    me->client = RKClientInitWithDesc(desc);

    RKClientSetUserResource(me->client, me);
    RKClientSetReceiveHandler(me->client, &RKPedestalPedzyRead);
    RKClientStart(me->client);

    return (RKPedestal)me;
}

int RKPedestalPedzyExec(RKPedestal input, const char *command, char *response) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClient *client = me->client;
    if (client->verbose > 1) {
        RKLog("%s received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        if (client->state < RKClientStateConnected) {
            if (response != NULL) {
                sprintf(response, "NAK. Pedestal not connected." RKEOL);
            }
            return RKResultIncompleteReceive;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumStringLength - 1, "%s" RKEOL, command);
        RKNetworkSendPackets(client->sd, me->latestCommand, size + 1, NULL);
        while (responseIndex == me->responseIndex) {
            usleep(10000);
            if (++s % 100 == 0) {
                RKLog("%s Waited %.2f for response.\n", client->name, (float)s * 0.01f);
            }
            if ((float)s * 0.01f >= 5.0f) {
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
    return RKResultNoError;
}

int RKPedestalPedzyFree(RKPedestal input) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultNoError;
}
