//
//  RKHealthRelayTweeta.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthRelayTweeta.h>

#pragma mark - Internal Functions

// Internal Implementations

int RHealthRelayTweetaRead(RKClient *client) {
    // The shared user resource pointer
    RKRadar *radar = client->userResource;

    // The payload just was just read by RKClient
    char *report = (char *)client->userPayload;
    RKStripTail(report);
    
    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        printf("%s\n", report);
    }
    
    // Get a vacant slot for health from Radar, copy over the data, then set it ready
    RKHealth *health = RKGetVacantHealth(radar);
    strncpy(health->string, report, RKMaximumStringLength - 1);   
    RKSetHealthReady(radar, health);
    
    return RKResultSuccess;
}

#pragma mark - Protocol Implementations

// Implementations

RKHealthRelay RKHealthRelayTweetaInit(RKRadar *radar, void *input) {
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)malloc(sizeof(RKHealthRelayTweeta));
    if (me == NULL) {
        RKLog("Error. Unable to allocated RKMonitorTweeta.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKHealthRelayTweeta));

    // Tweeta uses a TCP socket server at port 9556. The payload is always a line string terminated by \r\n
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    sprintf(desc.name, "%s<TweetaRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    strncpy(desc.hostname, (char *)input, RKMaximumStringLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9556;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatNewLine;
    desc.blocking = true;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose = 1;
    
    me->client = RKClientInitWithDesc(desc);
    
    RKClientSetUserResource(me->client, radar);
    RKClientSetReceiveHandler(me->client, &RHealthRelayTweetaRead);
    RKClientStart(me->client);
    
    return (RKHealthRelay)me;
}

int RKHealthRelayTweetaExec(RKHealthRelay input, const char *command, char *response) {
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)input;
    RKClient *client = me->client;
    if (client->verbose > 1) {
        RKLog("%s received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        RKNetworkSendPackets(client->sd, command, strlen(command), NULL);
    }
    return RKResultSuccess;
}

int RKHealthRelayTweetaFree(RKHealthRelay input) {
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultSuccess;
}
