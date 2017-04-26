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
    // The shared user resource pointer
    RKRadarRelay *engine = (RKRadarRelay *)client->userResource;

    char *string = client->userPayload;
    RKStripTail(string);
    RKLog("%s  %d/%d %s\n", engine->name, client->netDelimiter.type, client->netDelimiter.size, string);

    return RKResultSuccess;
}

#pragma mark - Delegate Workers

static void *radarRelay(void *in) {
    RKRadarRelay *engine = (RKRadarRelay *)in;
    //RKRadarDesc *desc = engine->radarDescription;

    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    strcpy(desc.name, engine->name);
    strncpy(desc.hostname, engine->host, RKNameLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 10000;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatHeaderDefinedSize;
    desc.blocking = true;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose = 1;

    engine->client = RKClientInitWithDesc(desc);

    RKClientSetUserResource(engine->client, engine);
    RKClientSetReceiveHandler(engine->client, &RKRadarRelayRead);
    RKClientStart(engine->client, false);
    
    RKLog("%s Started.   mem = %s B   host = %s\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), engine->host);

    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

    struct timeval t0, t1;

    gettimeofday(&t1, NULL);

    char cmd[RKNameLength];

    usleep(10000);
    size_t size = sprintf(cmd, "sh" RKEOL);
    printf("Sending command (%zu) ...\n", size);
    RKNetworkSendPackets(engine->client->sd, "sh" RKEOL, size, NULL);

    while (engine->state & RKEngineStateActive) {
        // Evaluate the nodal-health buffers every once in a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) < 0.5) {
            usleep(10000);
            continue;
        }
        t1 = t0;
    }

    RKClientStop(engine->client);

    return (void *)NULL;
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
    sprintf(engine->name, "%s<RadarRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorRadarRelay) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
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

void RKRadarRelaySetInputOutputBuffers(RKRadarRelay *engine, RKRadarDesc *desc, RKFileManager *fileManager,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKHealth *healthBuffer, uint32_t *healthIndex, const uint32_t healthBufferDepth,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth,
                                       RKBuffer rayBuffer,     uint32_t *rayIndex,    const uint32_t rayBufferDepth) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->configBufferDepth = configBufferDepth;
    engine->healthBuffer      = healthBuffer;
    engine->healthIndex       = healthIndex;
    engine->healthBufferDepth = healthBufferDepth;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->pulseBufferDepth  = pulseBufferDepth;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    engine->rayBufferDepth    = rayBufferDepth;
    engine->state |= RKEngineStateProperlyWired;
}

void RKRadarRelaySetHost(RKRadarRelay *engine, const char *hostname) {
    strncpy(engine->host, hostname, RKNameLength - 1);
}

#pragma mark - Interactions

int RKRadarRelayStart(RKRadarRelay *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;

    if (pthread_create(&engine->tidBackground, NULL, radarRelay, engine)) {
        RKLog("Error. Unable to start radar relay.\n");
        return RKResultFailedToStartHealthWorker;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKRadarRelayStop(RKRadarRelay *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose > 1) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    pthread_join(engine->tidBackground, NULL);
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->state);
    }
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
