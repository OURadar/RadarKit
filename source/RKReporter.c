//
//  RKReporter.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKReporter.h>

// Private declarations

static void disconnectStreams(RKReporter *engine) {
    if (engine->streamsInProgress & RKStreamHealthInJSON) {
        engine->streamsInProgress ^= RKStreamHealthInJSON;
        RKLog("%s Ended streaming RKStreamHealthInJSON ...\n", engine->name);
    }
    if (engine->streamsInProgress & RKStreamScopeStuff) {
        engine->streamsInProgress ^= RKStreamScopeStuff;
        RKLog("%s Ended streaming RKStreamScopeStuff ...\n", engine->name);
    }
    if (engine->streamsInProgress & RKStreamDisplayAll) {
        engine->streamsInProgress &= !RKStreamDisplayAll;
        RKLog("%s Ended streaming RKStreamDisplayAll ...\n", engine->name);
    }
}

#pragma mark - Busy Loop

void *reporter(void *in) {
    RKReporter *engine = (RKReporter *)in;
    RKRadar *radar = engine->radar;

    int h = 0;
    int p = 0;
    int r = 0;

    size_t payload_size;
    int count;

    char *message = engine->message;
    void *payload = engine->payload;

    int16_t *y;

    RKHealth *health;
    RKPulse *pulse;
    RKRay *ray;

    RKInt16C *xh, *xv;

    RKLog("%s Started.  mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));

    engine->state |= RKEngineStateActive;

    while (engine->state & RKEngineStateActive) {
        if (engine->ws->connected) {

            // Health Status
            #pragma mark Health Status

            if (engine->streams & RKStreamHealthInJSON) {
                if (!(engine->streamsInProgress & RKStreamHealthInJSON)) {
                    engine->streamsInProgress |= RKStreamHealthInJSON;
                    h = radar->healthIndex;
                    if (engine->verbose) {
                        RKLog("%s Begin streaming RKStreamHealthInJSON   h = %d\n", engine->name, h);
                    }
                }

                health = &radar->healths[h];

                if (health->flag & RKHealthFlagReady) {
                    if (h % engine->healthStride) {
                        usleep(1);
                    } else {
                        snprintf(payload, PAYLOAD_CAPACITY, "%c%s", RKRadarHubTypeHealth, health->string);
                        payload_size = 1 + strlen((char *)(payload + 1));
                        if (engine->verbose > 1) {
                            if (payload_size < 64) {
                                RKBinaryString(message, payload, payload_size);
                            } else {
                                RKHeadTailBinaryString(message, payload, payload_size);
                            }
                            RKLog("%s H%02d %s (%zu)", engine->name, h, message, payload_size);
                        }
                        RKWebSocketSend(engine->ws, payload, payload_size);
                    }
                    health->flag |= RKHealthFlagUsed;
                    h = RKNextModuloS(h, radar->desc.healthBufferDepth);
                }
            }

            // AScope Samples
            #pragma mark AScope Samples
            if (engine->streams & RKStreamScopeStuff) {
                if (!(engine->streamsInProgress & RKStreamScopeStuff)) {
                    engine->streamsInProgress |= RKStreamScopeStuff;
                    p = radar->pulseIndex;
                    if (engine->verbose) {
                        RKLog("%s Begin streaming RKStreamScopeStuff   p = %d\n", engine->name, p);
                    }
                }

                pulse = RKGetPulseFromBuffer(radar->pulses, p);

                if (pulse->header.s & RKPulseStatusReadyForMoments) {
                    if (p % engine->pulseStride) {
                        usleep(1);
                    } else if (engine->ws->connected) {
                        count = 100;
                        *(char *)payload = RKRadarHubTypeScope;
                        payload_size = sizeof(uint8_t) + 4 * count * sizeof(int16_t);
                        xh = RKGetInt16CDataFromPulse(pulse, 0);
                        xv = RKGetInt16CDataFromPulse(pulse, 1);
                        y = (int16_t *)(payload + 1);
                        for (int k = 0; k < count - 1; k++) {
                            *(y            ) = xh->i;
                            *(y + count    ) = xh->q;
                            *(y + count * 2) = xv->i;
                            *(y + count * 3) = xv->q;
                            xh++;
                            xv++;
                            y++;
                        }
                        *(y            ) = -1;
                        *(y + count    ) = 1;
                        *(y + count * 2) = -1;
                        *(y + count * 3) = 1;

                        if (engine->verbose > 1) {
                            RKLog("%s P%04d i=%06zu A%.2f\n", engine->name, p, pulse->header.i, pulse->header.azimuthDegrees);
                        }
                        RKWebSocketSend(engine->ws, payload, payload_size);
                    }
                    p = RKNextModuloS(p, radar->desc.pulseBufferDepth);
                }
            }

            // Product Display Data
            #pragma mark Product Display
            if (engine->streams & RKStreamDisplayAll) {
                if ((engine->streamsInProgress & RKStreamDisplayAll) != (engine->streams & RKStreamDisplayAll)) {
                    engine->streamsInProgress |= (engine->streams & RKStreamDisplayAll);
                    r = radar->rayIndex;
                    if (engine->verbose) {
                        RKLog("%s Begin streaming RKStreamDisplay   r = %d\n", engine->name, r);
                    }
                }

                ray = RKGetRayFromBuffer(radar->rays, r);

                if (r % engine->rayStride) {
                    ray->header.s |= RKRayStatusStreamed;
                    r = RKNextModuloS(p, radar->desc.pulseBufferDepth);
                } else if (ray->header.s & RKRayStatusReady) {
                    ray->header.s |= RKRayStatusStreamed;
                    r = RKNextModuloS(p, radar->desc.pulseBufferDepth);
                } else if (engine->verbose > 2) {
                    RKLog("%s Ray not ready   r = %d   radar->rayIndex = %d\n", engine->name, r, radar->rayIndex);
                }
            }

        } else {

            disconnectStreams(engine);

        }
        usleep(1000);
    }

    disconnectStreams(engine);

    return NULL;
}

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

void handleOpen(RKWebSocket *w) {
    RKReporter *engine = (RKReporter *)w->parent;
    size_t r = sprintf(engine->welcome,
        "%c{"
            "\"command\":\"radarConnect\", "
            "\"pathway\":\"%s\", "
            "\"name\":\"%s\""
        "}",
        RKRadarHubTypeHandshake, engine->pathway, engine->radar->desc.name);
    RKLog("%s Sending open packet %s ...\n", engine->name, engine->address);
    if (engine->verbose) {
        RKLog("%s %s\n", engine->name, engine->welcome);
    }
    RKWebSocketSend(w, engine->welcome, r);
//    sendControl(w);
}

void handleClose(RKWebSocket *W) {
//    R->connected = false;
//    if (R->verbose) {
//        printf("ONCLOSE\n");
//    }
    RKReporter *engine = (RKReporter *)W->parent;
    if (engine->state & RKEngineStateActive) {
        engine->state ^= RKEngineStateActive;
    }
    RKLog("%s WebSocket handleClose()\n", engine->name);
}

void handleMessage(RKWebSocket *W, void *payload, size_t size) {
    RKReporter *engine = (RKReporter *)W->parent;
    RKRadar *radar = engine->radar;

    char *message = (char *)payload;
    RKLog("%s radar = %s   %s\n", engine->name, radar->desc.name, message);
}


#pragma mark - Life Cycle

RKReporter *RKReporterInitWithHost(const char *host) {
    RKReporter *engine = (RKReporter *)malloc(sizeof(RKReporter));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate an RKReporter.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKReporter));
    sprintf(engine->name, "%s<RadarHubConnect>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorRadarHubReporter) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->healthStride = 2;
    engine->pulseStride = 100;
    engine->rayStride = 1;
    engine->memoryUsage = sizeof(RKReporter);
    strncpy(engine->host, host, RKNameLength);

    // Could move most of these to RKWebScoket.c
    RKLog("%s host = %s\n", engine->name, engine->host);
    if (strstr(engine->host, "https") != NULL) {
        engine->flag = RKWebSocketFlagSSLOn;
    } else {
        engine->flag = RKWebSocketFlagSSLOff;
    }
    char *c = strstr(engine->host, "://");
    if (c) {
        size_t l = strlen(engine->host) - (size_t)(c - engine->host) - 3;
        memmove(engine->host, c + 3, l);
        engine->host[l] = '\0';
    }
    if (engine->flag & RKWebSocketFlagSSLOn && strstr(engine->host, ":") == NULL) {
        strcat(engine->host, ":443");
    }
    if (strstr(engine->host, ":443")) {
        engine->flag = RKWebSocketFlagSSLOn;
    }
    RKLog("%s revised host = %s (SSL %s)\n", engine->name, engine->host, engine->flag & RKWebSocketFlagSSLOn ? "On" : "Off");

    // Default streams
    engine->streams = RKStreamHealthInJSON | RKStreamScopeStuff;
    return engine;
}

RKReporter *RKReporterInitForRadarHub(void) {
//    return RKReporterInitWithHost("https://radarhub.arrc.ou.edu");
//    return RKReporterInitWithHost("radarhub.arrc.ou.edu:443");
    return RKReporterInitWithHost("10.197.14.52:8001");
}

RKReporter *RKReporterInitForLocal(void) {
    return RKReporterInitWithHost("http://localhost:8001");
}

RKReporter *RKReporterInit(void) {
    return RKReporterInitForRadarHub();
}

void RKReporterFree(RKReporter *engine) {
    free(engine);
}

#pragma mark - Properties

void RKReporterSetVerbose(RKReporter *engine, const int verbose) {
    engine->verbose = verbose;
    if (engine->ws) {
        RKWebSocketSetVerbose(engine->ws, engine->verbose);
    }
}

void RKReporterSetRadar(RKReporter *engine, RKRadar *radar) {
    engine->radar = radar;
    strcpy(engine->pathway, radar->desc.name);
    RKStringLower(engine->pathway);
    sprintf(engine->address, "/ws/radar/%s/", engine->pathway);
    RKLog("%s Setting up radar %s @ %s\n", engine->name, radar->desc.name, engine->address);
    engine->ws = RKWebSocketInit(engine->host, engine->address, engine->flag);
    RKWebSocketSetOpenHandler(engine->ws, &handleOpen);
    RKWebSocketSetCloseHandler(engine->ws, &handleClose);
    RKWebSocketSetMessageHandler(engine->ws, &handleMessage);
    RKWebSocketSetVerbose(engine->ws, engine->verbose);
    RKWebSocketSetParent(engine->ws, engine);
}

#pragma mark - Interactions

void RKReporterStart(RKReporter *engine) {
    RKLog("%s Starting %s:%s ...\n", engine->name, engine->ws->host, engine->ws->path);
    RKWebSocketStart(engine->ws);
    if (pthread_create(&engine->ticWorker, NULL, reporter, engine)) {
        RKLog("%s Error. Failed to create reporter().\n", engine->name);
        return;
    }
}

void RKReporterStop(RKReporter *engine) {
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    RKLog("%s Disconnecting %s:%s ...\n", engine->name, engine->ws->host, engine->ws->path);
    RKWebSocketStop(engine->ws);
    RKLog("%s Stopped.\n", engine->name);
}
