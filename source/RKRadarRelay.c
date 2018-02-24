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

    int j, k;
    RKHealth *health;
    RKStatus *status;
    RKRay *ray = engine->rayBuffer;
    RKPulse *pulse = engine->pulseBuffer;

    uint8_t *u8Data = NULL;
    uint32_t productList;
    uint32_t productCount;
    uint32_t rxGateCount = 0;

    const uint32_t localRayCapacity = ray->header.capacity;
    const uint32_t localPulseCapacity = pulse->header.capacity;

    RKInt16C *c16DataH = NULL;
    RKInt16C *c16DataV = NULL;
    RKPulseStatus pulseStatus = RKPulseStatusVacant;
    uint32_t pulseSize = 0;

    switch (client->netDelimiter.type) {
        case RKNetworkPacketTypeBeacon:
            // Ignore beacon
            if (engine->verbose > 1) {
                RKLog("%s beacon.\n", engine->name);
            }
            break;
            
        case RKNetworkPacketTypeProcessorStatus:
            // Queue up the status
            k = *engine->statusIndex;
            status = &engine->statusBuffer[k];
            ((RKStatus *)(client->userPayload))->flag = status->flag;
            memcpy(status, client->userPayload, sizeof(RKStatus));
            status->flag = RKStatusFlagReady;
            k = RKNextModuloS(k, engine->radarDescription->statusBufferDepth);
            status = &engine->statusBuffer[k];
            status->flag = RKStatusFlagVacant;
            *engine->statusIndex = k;
            if (engine->verbose > 1) {
                RKLog("%s RKNetworkPacketTypeProcessorStatus: %llu\n", engine->name, k);
            }
            break;
            
        case RKNetworkPacketTypeHealth:
            // Queue up the health
            k = *engine->healthIndex;
            health = &engine->healthBuffer[k];
            ((RKHealth *)(client->userPayload))->flag = health->flag;
            strncpy(health->string, client->userPayload, RKMaximumStringLength);
            health->flag = RKHealthFlagReady;
            //k = RKNextModuloS(k, engine->healthBufferDepth);
            k = RKNextModuloS(k, engine->radarDescription->healthBufferDepth);
            health = &engine->healthBuffer[k];
            health->string[0] = '\0';
            health->flag = RKHealthFlagVacant;
            *engine->healthIndex = k;
            if (engine->verbose > 1) {
                RKLog("%s RKNetworkPacketTypeHealth: %llu\n", engine->name, k);
            }
            break;

        case RKNetworkPacketTypeRayData:
            break;

        case RKNetworkPacketTypePulseData:
            // Override the status of the payload
            pulse = (RKPulse *)client->userPayload;
            pulseStatus = pulse->header.s;
            //rxGateCount = pulse->header.capacity;

            //printf("%s Pulse packet -> %d (remote/local capacity %d / %d).\n", engine->name, *engine->pulseIndex, pulse->header.capacity, localPulseCapacity);

            pulse->header.capacity = localPulseCapacity;
            if (pulse->header.gateCount > pulse->header.capacity) {
                pulse->header.gateCount = pulse->header.capacity;
            }
            pulse->header.s |= RKPulseStatusInspected;

            // Now we get a slot to fill it in
            pulse = RKGetPulse(engine->pulseBuffer, *engine->pulseIndex);
            memcpy(&pulse->header, client->userPayload, sizeof(RKPulseHeader));

            pulseSize = pulse->header.gateCount * sizeof(RKInt16C);

            c16DataH = RKGetInt16CDataFromPulse(pulse, 0);
            c16DataV = RKGetInt16CDataFromPulse(pulse, 1);

            memcpy(c16DataH, client->userPayload + sizeof(RKPulseHeader), pulseSize);
            memcpy(c16DataV, client->userPayload + sizeof(RKPulseHeader) + pulseSize, pulseSize);

            pulse->header.s = pulseStatus;

            *engine->pulseIndex = RKNextModuloS(*engine->pulseIndex, engine->radarDescription->pulseBufferDepth);
            pulse = RKGetPulse(engine->pulseBuffer, *engine->pulseIndex);
            pulse->header.s = RKPulseStatusVacant;
            break;

        case RKNetworkPacketTypeRayDisplay:
            // Override the status of the payload
            ray = (RKRay *)client->userPayload;
            rxGateCount = ray->header.gateCount;

            //printf("%s Display packet -> %d (remote/local capacity %d / %d).\n", engine->name, *engine->rayIndex, ray->header.capacity, localRayCapacity);

            ray->header.capacity = localRayCapacity;
            if (ray->header.gateCount > localRayCapacity) {
                ray->header.gateCount = localRayCapacity;
            }
            ray->header.s = RKRayStatusProcessing;

            // Now we get a slot to fill it in
            ray = RKGetRay(engine->rayBuffer, *engine->rayIndex);
            memcpy(&ray->header, client->userPayload, sizeof(RKRayHeader));

            productList = ray->header.productList;
            productCount = __builtin_popcount(productList);
            for (j = 0; j < productCount; j++) {
                if (productList & RKProductListDisplayZ) {
                    productList ^= RKProductListDisplayZ;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
                } else if (productList & RKProductListDisplayV) {
                    productList ^= RKProductListDisplayV;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexV);
                } else if (productList & RKProductListDisplayW) {
                    productList ^= RKProductListDisplayW;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexW);
                } else if (productList & RKProductListDisplayD) {
                    productList ^= RKProductListDisplayD;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexD);
                } else if (productList & RKProductListDisplayP) {
                    productList ^= RKProductListDisplayP;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexP);
                } else if (productList & RKProductListDisplayR) {
                    productList ^= RKProductListDisplayR;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexR);
                } else if (productList & RKProductListDisplayK) {
                    productList ^= RKProductListDisplayK;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexK);
                } else if (productList & RKProductListDisplayS) {
                    productList ^= RKProductListDisplayS;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexS);
                } else {
                    u8Data = NULL;
                }
                
                if (u8Data) {
                    memcpy(u8Data, client->userPayload + sizeof(RKRayHeader) + j * rxGateCount * sizeof(uint8_t), ray->header.gateCount * sizeof(uint8_t));
                }
            }
            ray->header.s = RKRayStatusProcessed | RKRayStatusReady;

            *engine->rayIndex = RKNextModuloS(*engine->rayIndex, engine->radarDescription->rayBufferDepth);
            ray = RKGetRay(engine->rayBuffer, *engine->rayIndex);
            ray->header.s = RKRayStatusVacant;
            break;
            
        case RKNetworkPacketTypePlainText:
            break;

        case RKNetworkPacketTypeCommandResponse:
        case RKNetworkPacketTypeControls:
            // Queue up the feedback
            strncpy(engine->responses[engine->responseIndex], client->userPayload, RKRadarRelayFeedbackCapacity - 1);
            engine->responseIndex = RKNextModuloS(engine->responseIndex, RKRadarRelayFeedbackDepth);
            break;
            
        default:
            RKLog("%s New type %d of size %s\n", engine->name, client->netDelimiter.type, RKIntegerToCommaStyleString(client->netDelimiter.size));
            break;
    }

    return RKResultSuccess;
}

#pragma mark - Delegate Workers

static void *radarRelay(void *in) {
    RKRadarRelay *engine = (RKRadarRelay *)in;

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

    RKLog("%s Started.   mem = %s B   host = %s\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), engine->host);

    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

    struct timeval t0, t1;

    gettimeofday(&t1, NULL);

    char cmd[RKNameLength];

    RKClientStart(engine->client, true);

    uint32_t size = sprintf(cmd, "a RKRelay nopassword" RKEOL);
    pthread_mutex_lock(&engine->client->lock);
    RKNetworkSendPackets(engine->client->sd, cmd, size, NULL);
    pthread_mutex_unlock(&engine->client->lock);

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

void RKRadarRelaySetInputOutputBuffers(RKRadarRelay *engine, const RKRadarDesc *desc, RKFileManager *fileManager,
                                       RKStatus *statusBuffer, uint32_t *statusIndex,
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKHealth *healthBuffer, uint32_t *healthIndex,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,
                                       RKBuffer rayBuffer,     uint32_t *rayIndex) {
    engine->radarDescription  = (RKRadarDesc *)desc;
    engine->fileManager       = fileManager;
    engine->statusBuffer      = statusBuffer;
    engine->statusIndex       = statusIndex;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->healthBuffer      = healthBuffer;
    engine->healthIndex       = healthIndex;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
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
    RKClientStop(engine->client);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    pthread_join(engine->tidBackground, NULL);
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

int RKRadarRelayExec(RKRadarRelay *engine, const char *command, char *response) {
    RKClient *client = engine->client;
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
        uint32_t responseIndex = engine->responseIndex;
        uint32_t size = snprintf(engine->latestCommand, RKMaximumStringLength - 1, "%s" RKEOL, command);
        RKNetworkSendPackets(client->sd, engine->latestCommand, size, NULL);
        while (responseIndex == engine->responseIndex) {
            usleep(10000);
            if (++s % 100 == 0) {
                RKLog("%s Waited %.2f s for response.\n", client->name, (float)s * 0.01f);
            }
            if ((float)s * 0.01f >= 3.0f) {
                RKLog("%s should time out.\n", client->name);
                break;
            }
        }
        if (responseIndex == engine->responseIndex) {
            if (response != NULL) {
                sprintf(response, "NAK. Timeout." RKEOL);
            }
            return RKResultTimeout;
        }
        if (response != NULL) {
            strcpy(response, engine->responses[responseIndex]);
        }
    }
    return RKResultSuccess;
}
