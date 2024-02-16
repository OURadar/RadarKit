//
//  RKHealthRelayNaveen.c
//  RadarKit
//
//  Created by Boonleng Cheong on 6/9/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthRelayNaveen.h>
#include "ext/nmea.c"

// Internal Functions

static int healthRelayNaveenRead(RKClient *client) {
    // The shared user resource pointer
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)client->userResource;
    RKRadar *radar = me->radar;

    static nmea_data_t nmea = {
        .utc_time = 0,
        .valid = false
    };
    int r = nmea_parse_sentence(&nmea, (char *)client->userPayload);
    if (r) {
        return r;
    }
    #if defined(DEBUG_NAVEEN)
    printf("valid = %d   utc_time = %f   (%.6f, %.6f) @ %.2f\n", nmea.valid, nmea.utc_time, nmea.longitude, nmea.latitude, nmea.heading);
    #endif
    // Get a vacant slot for health from Radar, copy over the data, then set it ready
    RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeUser1);
    if (health == NULL) {
        RKLog("%s failed to get a vacant health.\n", client->name);
        return RKResultFailedToGetVacantHealth;
    }
    RKStatusEnum e = nmea.valid ? RKStatusEnumNormal : RKStatusEnumInvalid;
    snprintf(health->string, RKMaximumStringLength, "{"
        "\"GPS Valid\": {\"Value\":%s,\"Enum\":%d}, "
        "\"GPS Heading\": {\"Value\":%.1f,\"Enum\":%d}, "
        "\"GPS Latitude\": {\"Value\":%.7f,\"Enum\":%d}, "
        "\"GPS Longitude\": {\"Value\":%.7f,\"Enum\":%d}, "
        "\"Ground Speed\":{\"Value\":\"%.2f km/h\",\"Enum\":%d}"
        "}",
        nmea.valid ? "true" : "false", e,
        nmea.heading, e,
        nmea.latitude, e,
        nmea.longitude, e,
        nmea.speed, e
    );
    RKSetHealthReady(radar, health);
    return RKResultSuccess;
}

static int healthRelayNaveenGreet(RKClient *client) {
    ssize_t s = RKNetworkSendPackets(client->sd, "\n", 1, NULL);
    if (s < 0) {
        return (int)-s;
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
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.reconnect = true;
    desc.ping = false;
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

    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        if (client->verbose) {
            RKLog("%s Current client->state = 0x%08x   command = '%s'", client->name, client->state, command);
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
