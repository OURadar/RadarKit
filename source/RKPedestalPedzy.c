//
//  RKPedestalPedzy.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/4/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPedestalPedzy.h>

int RKPedestalPedzyRead(RKClient *client);

int RKPedestalPedzyRead(RKClient *client) {
    RKRadar *radar = client->userResource;

    RKPosition *position = (RKPosition *)client->userPayload;

    RKPosition *newPosition = RKGetVacantPosition(radar);
    memcpy(newPosition, position, sizeof(RKPosition));
    RKSetPositionReady(newPosition);

    return 0;
}

RKPedestal RKPedestalPedzyInit(RKRadar *radar, void *input) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)malloc(sizeof(RKPedestalPedzy));
    if (me == NULL) {
        RKLog("Error. Unable to allocated RKPedestalPedzy.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKPedestalPedzy));
    
    RKClientDesc desc;
    strncpy(desc.hostname, (char *)input, RKMaximumStringLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9000;
    }
    strcpy(desc.name, "<RKPedestalPedzy>");
    desc.format = RKMessageFormatFixedBlock;
    desc.blockLength = sizeof(RKPosition);
    desc.blocking = true;
    desc.reconnect = true;
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
    RKNetworkSendPackets(client->sd, command, strlen(command), NULL);
    return RKResultNoError;
}

int RKPedestalPedzyFree(RKPedestal input) {
    RKPedestalPedzy *me = (RKPedestalPedzy *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultNoError;
}

