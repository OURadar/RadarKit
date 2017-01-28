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
    RKRadar *radar = client->userResource;

    // The payload just was just read by RKClient
    RKPosition *position = (RKPosition *)client->userPayload;

    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("Position %08x EL %.2f  AZ %.2f --> %d\n", position->flag, position->elevationDegrees, position->azimuthDegrees, *radar->positionEngine->positionIndex);
    }
    
    // Get a vacant slot for position from Radar, copy over the data, then set it ready
    RKPosition *newPosition = RKGetVacantPosition(radar);
    if (newPosition == NULL) {
        RKLog("%s failed to get a vacant position.\n", client->name);
        return RKResultFailedToGetVacantPosition;
    }
    memcpy(newPosition, position, sizeof(RKPosition));
    RKSetPositionReady(radar, newPosition);

    return RKResultNoError;
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
    
    // Pedzy uses a TCP socket server at port 9000. The payload is always sizeof(RKPosition)
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
        desc.port = 9000;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatConstantSize;
    desc.blocking = true;
    desc.reconnect = true;
    desc.blockLength = sizeof(RKPosition);
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose = 1;
    
    me->client = RKClientInitWithDesc(desc);

    RKClientSetUserResource(me->client, radar);
    RKClientSetReceiveHandler(me->client, &RKPedestalPedzyRead);
    RKClientStart(me->client);

    return (RKPedestal)me;
}

int RKPedestalPedzyExec(RKPedestal input, const char *command) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClient *client = me->client;
    if (client->verbose > 1) {
        RKLog("%s received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKLog("RKPedestalPedzyExec() disconnect %d", client->state);
        RKClientStop(client);
    } else {
        RKNetworkSendPackets(client->sd, command, strlen(command), NULL);
    }
    return RKResultNoError;
}

int RKPedestalPedzyFree(RKPedestal input) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultNoError;
}
