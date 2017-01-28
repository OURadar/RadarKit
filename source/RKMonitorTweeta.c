//
//  RKMonitorTweeta.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKMonitorTweeta.h>

#pragma mark - Internal Functions

// Internal Implementations

int RKMonitorTweetaRead(RKClient *client) {
    return 0;
}

#pragma mark - Protocol Implementations

// Implementations

RKHealthMonitor RKHealthMonitorTweetaInit(RKRadar *radar, void *input) {
    RKHealthMonitorTweeta *me = (RKHealthMonitorTweeta *)malloc(sizeof(RKHealthMonitorTweeta));
    if (me == NULL) {
        RKLog("Error. Unable to allocated RKMonitorTweeta.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKHealthMonitorTweeta));

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
    desc.format = RKNetworkMessageFormatConstantSize;
    desc.blocking = true;
    desc.reconnect = true;
    desc.blockLength = sizeof(RKPosition);
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose = 1;
    
    me->client = RKClientInitWithDesc(desc);
    
    RKClientSetUserResource(me->client, radar);
    RKClientSetReceiveHandler(me->client, &RKMonitorTweetaRead);
    RKClientStart(me->client);
    
    return (RKHealthMonitor)me;
}

int RKHealthMonitorTweetaExec(RKHealthMonitor input, const char *command) {
    RKHealthMonitorTweeta *me = (RKHealthMonitorTweeta *)input;
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

int RKHealthMonitorTweetaFree(RKHealthMonitor input) {
    RKHealthMonitorTweeta *me = (RKHealthMonitorTweeta *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultSuccess;
}
