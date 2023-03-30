//
//  RKHealthRelayTweeta.c
//  RadarKit
//
//  Created by Boonleng Cheong on 1/28/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthRelayTweeta.h>

#pragma mark - Internal Functions

static int healthRelayTweetaRead(RKClient *client) {
    // The shared user resource pointer
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)client->userResource;
    RKRadar *radar = me->radar;

    char *string = (char *)client->userPayload;
    char *stringValue;

    if (client->netDelimiter.type == 's') {
        // The payload just read by RKClient
        if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
            printf("%s\n", string);
        }

        // Get a vacant slot for health from Radar, copy over the data, then set it ready
        RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeTweeta);
        if (health == NULL) {
            RKLog("%s failed to get a vacant health.\n", client->name);
            return RKResultFailedToGetVacantHealth;
        }
        strncpy(health->string, string, RKMaximumStringLength - 1);
        RKStripTail(health->string);

        // Handle a special event from a push button
        if ((stringValue = RKGetValueOfKey(health->string, "event")) != NULL && !strcasecmp(stringValue, "short")) {
            RKLog("%s event %s\n", client->name, stringValue);
            RKPerformMasterTaskInBackground(radar, "b");
        }

        RKSetHealthReady(radar, health);
    } else {
        // This the command acknowledgement, queue it up to feedback
        if (!strncmp(string, "pong", 4)) {
            // Just a beacon response.
        } else {
            strncpy(me->responses[me->responseIndex], client->userPayload, RKMaximumStringLength - 1);
            me->responseIndex = RKNextModuloS(me->responseIndex, RKHealthRelayTweetaFeedbackDepth);
            if (client->verbose > 1 && me->latestCommand[0] != 'h') {
                RKStripTail(string);
                RKLog("%s %s (delim.type = %d)\n", client->name, string, client->netDelimiter.type);
            }
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
    sprintf(desc.name, "%s<TweetaAutoRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHealthRelayTweeta) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    strncpy(desc.hostname, (char *)input, RKNameLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9556;
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
    RKClientSetReceiveHandler(me->client, &healthRelayTweetaRead);
    RKClientStart(me->client, false);

    return (RKHealthRelay)me;
}

int RKHealthRelayTweetaExec(RKHealthRelay input, const char *command, char *response) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)input;
    RKClient *client = me->client;
    if (client->verbose > 1) {
        RKLog("%s Received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        if (client->state != RKClientStateConnected) {
            RKLog("%s Health Relay not connected for command '%s'.\n", client->name, command);
            if (response != NULL) {
                sprintf(response, "NAK. Health Relay not connected." RKEOL);
            }
            return RKResultClientNotConnected;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumCommandLength - 1, "%s" RKEOL, command);
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
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultSuccess;
}
