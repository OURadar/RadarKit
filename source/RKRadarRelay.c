//
//  RKRadarRelay.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 4/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKRadarRelay.h>

#pragma mark - Internal Functions

static int RKRadarRelayRead(RKClient *client) {
    
    return RKResultSuccess;
}

#pragma mark - Life Cycle

RKRadarRelay *RKRadarRelayInit(void) {
    RKRadarRelay *engine = (RKRadarRelay *)malloc(sizeof(RKRadarRelay));
    if (engine == NULL) {
        RKLog("Error. Unable to allocated a radar relay.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKRadarRelay));
    
    // TCP socket server over port 10000.
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    sprintf(engine->name, "%s<RadarRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(14) : "", rkGlobalParameters.showColor ? RKNoColor : "");
//    strncpy(desc.hostname, (char *)input, RKMaximumStringLength - 1);
//    char *colon = strstr(desc.hostname, ":");
//    if (colon != NULL) {
//        *colon = '\0';
//        sscanf(colon + 1, "%d", &desc.port);
//    } else {
//        desc.port = 10000;
//    }
//    desc.type = RKNetworkSocketTypeTCP;
//    desc.format = RKNetworkMessageFormatHeaderDefinedSize;
//    desc.blocking = true;
//    desc.reconnect = true;
//    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
//    desc.verbose = 2;
//    
//    engine->client = RKClientInitWithDesc(desc);
//    
//    RKClientSetUserResource(engine->client, engine);
//    RKClientSetReceiveHandler(engine->client, &RKRadarRelayRead);
//    RKClientStart(engine->client, false);
    engine->state = RKEngineStateAllocated;

    return (RKRadarRelay *)engine;
}

void RKRadarRelayFree(RKRadarRelay *engine) {
    free(engine);
}

#pragma mark - Properties

void RKRadarRelaySetVerbose(RKRadarRelay *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKRadarRelaySetHost(RKRadarRelay *engine, const char *hostname) {
    strncpy(engine->host, hostname, RKNameLength - 1);
}

#pragma mark - Interactions

int RKRadarRelayStart(RKRadarRelay *engine) {
    return RKResultSuccess;
}

int RKRadarRelayStop(RKRadarRelay *engine) {
    return RKResultSuccess;
}

int RKRadarRelayExec(RKMasterController input, const char *command, char *response) {
    RKRadarRelay *me = (RKRadarRelay *)input;
    RKClient *client = me->client;
    if (client->verbose > 1) {
        RKLog("%s received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        if (client->state < RKClientStateConnected) {
            if (response != NULL) {
                sprintf(response, "NAK. Radar Relay not connected." RKEOL);
            }
            return RKResultIncompleteReceive;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumStringLength - 1, "%s" RKEOL, command);
        RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        while (responseIndex == me->responseIndex) {
            usleep(10000);
            if (++s % 100 == 0) {
                RKLog("%s Waited %.2f s for response.\n", client->name, (float)s * 0.01f);
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
