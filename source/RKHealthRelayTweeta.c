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
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)client->userResource;
    RKRadar *radar = me->radar;

    if (client->netDelimiter.type == 's') {
        // The payload just read by RKClient
        char *report = (char *)client->userPayload;
        RKStripTail(report);

        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            printf("%s\n", report);
        }

        // Get a vacant slot for health from Radar, copy over the data, then set it ready
        RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeTweeta);
        if (health == NULL) {
            RKLog("%s failed to get a vacant health.\n", client->name);
            return RKResultFailedToGetVacantHealth;
        }
        strncpy(health->string, report, RKMaximumStringLength - 1);
        RKSetHealthReady(radar, health);
    } else {
        // This the command acknowledgement, queue it up to feedback
        char *string = (char *)client->userPayload;
        if (!strncmp(string, "pong", 4)) {
            // Just a beacon response.
        } else {
            if (client->verbose && me->latestCommand[0] != 'h') {
                RKLog("%s (type %d) %s", client->name, client->netDelimiter.type, string);
            }
            strncpy(me->responses[me->responseIndex], client->userPayload, RKMaximumStringLength - 1);
            me->responseIndex = RKNextModuloS(me->responseIndex, RKHealthRelayTweetaFeedbackDepth);
        }
    }

    return RKResultSuccess;
}

#pragma mark - Protocol Implementations

// Implementations

RKHealthRelay RKHealthRelayTweetaInit(RKRadar *radar, void *input) {
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)malloc(sizeof(RKHealthRelayTweeta));
    if (me == NULL) {
        RKLog("Error. Unable to allocated RKHealthRelayTweeta.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKHealthRelayTweeta));
    me->radar = radar;

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
        if (client->state < RKClientStateConnected) {
            if (response != NULL) {
                sprintf(response, "NAK. Health Relay not connected." RKEOL);
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
    return RKResultSuccess;
}

int RKHealthRelayTweetaFree(RKHealthRelay input) {
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultSuccess;
}
