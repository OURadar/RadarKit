//
//  RKHealthRelayNaveen.c
//  RadarKit
//
//  Created by Boonleng Cheong on 6/9/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthRelayNaveen.h>

// Internal Functions

static int healthRelayNaveenRead(RKClient *client) {
    // The shared user resource pointer
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)client->userResource;
    RKRadar *radar = me->radar;

    char *string = (char *)client->userPayload;
    char *stringValue;

    printf("%s\n", string);

    return RKResultSuccess;
}

static int healthRelayNaveenGreet(RKClient *client) {
    ssize_t s = RKNetworkSendPackets(client->sd, "\n", 1, NULL);
    if (s < 0) {
        return -s;
    }
    RKLog("%s Naveen @ %s:%d connected.\n", client->name, client->hostIP, client->port);
    return RKResultSuccess;
}

#pragma mark - Protocol Implementations

// Implementations
RKHealthRelay RKHealthRelayNaveenInit(RKRadar *radar, void *input) {
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)malloc(sizeof(RKHealthRelayNaveen));
     if (me == NULL) {
        RKLog("Error. Unable to allocated RKHealthRelayNaveen.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKHealthRelayNaveen));
    me->radar = radar;

    // What the server is uses a TCP socket at default port 9557. The payload is aywas a line string terminated by \r\n
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    sprintf(desc.name, "%s<NaveenAutoRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHealthRelayNaveen) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    strncpy(desc.hostname, (char *)input, RKNameLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9557;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatNewLine;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose =
    radar->desc.initFlags & RKInitFlagVeryVeryVerbose ? 3 :
    (radar->desc.initFlags & RKInitFlagVeryVerbose ? 2:
     (radar->desc.initFlags & RKInitFlagVerbose ? 1 : 0));

    me->client = RKClientInitWithDesc(desc);
    RKClientSetUserResource(me->client, me);
    RKClientSetGreetHandler(me->client, &healthRelayNaveenGreet);
    RKClientSetReceiveHandler(me->client, &healthRelayNaveenRead);
    RKClientStart(me->client, false);

   return (RKHealthRelay)me;
}

int RKHealthRelayNaveenExec(RKHealthRelay input, const char * command, char _Nullable *response) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)input;
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
            RKLog("%s Health Relay not connected for command '%s'.\n", client->name, command);
            sprintf(response, "NAK. Health Relay not connected." RKEOL);
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

int RKHealthRelayNaveenFree(RKHealthRelay input) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)input;
    RKClientFree(me->client);
    free(me);
    return RKResultSuccess;
}
