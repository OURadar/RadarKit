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
        engine->streamsInProgress &= ~RKStreamDisplayAll;
        RKLog("%s Ended streaming RKStreamDisplayAll ...\n", engine->name);
    }
}

#pragma region Busy Loop

void *reporter(void *in) {
    RKReporter *engine = (RKReporter *)in;
    RKRadar *radar = engine->radar;

    const double scopeRefreshPeriod = 1.0 / 20.0;

    int j, k;
    int count;
    int stride;
    int d = 0;
    int h = 0;
    int p = 0;
    int r = 0;

    uint32_t controlUID = (uint32_t)-1;
    uint16_t rayConfigIndex = (uint16_t)-1;
    size_t payloadSize = 0, cummulativePayloadSize = 0;
    double t0, t1 = 0.0;
    double n0, n1 = 0.0;
    float rate = 0.0f;

    char *message = engine->message;
    void *payload = NULL;

    int16_t *y;
    uint8_t *z;

    RKConfig *config;
    RKHealth *health;
    RKPulse *pulse;
    RKRay *ray;

    RKInt16C *xh, *xv;

    RKRadarHubRay *display;

    struct timeval tv;

    do {
        usleep(10000);
    } while (engine->ws->tic < 1);

    RKLog("%s Started.  mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));

    engine->state |= RKEngineStateActive;

    gettimeofday(&tv, NULL);
    t1 = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
    n1 = t1;

    while (engine->state & RKEngineStateWantActive) {
        if (engine->ws->connected) {

            // Controls
            #pragma region Controls
            if (controlUID != radar->controls[0].uid) {
                RKLog("%s Control UID changed from %u to %u (%d)\n", engine->name, controlUID, radar->controls[0].uid, radar->controlCount);
                controlUID = radar->controls[0].uid;
                RKMakeJSONStringFromControls(engine->scratch, radar->controls, radar->controlCount);
                r = snprintf(engine->control, RKMaximumStringLength,
                    "%c{"
                        "\"pathway\": \"%s\", "
                        "\"control\": [%s]"
                    "}",
                    RKRadarHubTypeControl, engine->pathway, engine->scratch);
                RKWebSocketSend(engine->ws, engine->control, strlen(engine->control));
                // printf("%s control = %s\n", engine->name, engine->control);
            }

            // Health Status
            #pragma region Health Status
            if (engine->streams & RKStreamHealthInJSON) {
                if (!(engine->streamsInProgress & RKStreamHealthInJSON)) {
                    engine->streamsInProgress |= RKStreamHealthInJSON;
                    h = radar->healthIndex;
                    if (engine->verbose) {
                        RKLog("%s RKStreamHealthInJSON started.   h = %d\n", engine->name, h);
                    }
                }

                health = &radar->healths[h];

                if (health->flag & RKHealthFlagReady) {
                    if (h % engine->healthStride) {
                        usleep(1);
                    } else {
                        // Use payload 0 as target
                        payload = engine->payload[0];
                        // Put together the health payload for RadarHub
                        snprintf(payload, PAYLOAD_CAPACITY, "%c%s", RKRadarHubTypeHealth, health->string);
                        payloadSize = 1 + strlen((char *)(payload + 1));
                        if (engine->verbose > 1) {
                            if (payloadSize < 64) {
                                RKBinaryString(message, payload, payloadSize);
                            } else {
                                RKHeadTailBinaryString(message, payload, payloadSize);
                            }
                            RKLog("%s H%04d %s (%zu)", engine->name, h, message, payloadSize);
                        }
                        RKWebSocketSend(engine->ws, payload, payloadSize);
                        cummulativePayloadSize += payloadSize;
                    }
                    health->flag |= RKHealthFlagUsed;
                    h = RKNextModuloS(h, radar->desc.healthBufferDepth);
                }
            }

            // AScope Samples
            #pragma region AScope Samples
            if (engine->streams & RKStreamScopeStuff) {
                if (!(engine->streamsInProgress & RKStreamScopeStuff)) {
                    engine->streamsInProgress |= RKStreamScopeStuff;
                    p = radar->pulseIndex;
                    if (engine->verbose) {
                        RKLog("%s RKStreamScopeStuff started.   p = %d\n", engine->name, p);
                    }
                }

                pulse = RKGetPulseFromBuffer(radar->pulses, p);

                if (pulse->header.s & RKPulseStatusHasConfigIndex) {
                    gettimeofday(&tv, NULL);
                    t0 = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
                    if (p % engine->pulseStride || (t0 - t1) < scopeRefreshPeriod) {
                        usleep(1);
                    } else {
                        // Use payload 1 as target
                        payload = engine->payload[1];
                        // Put together the payload for RadarHub
                        config = &radar->configs[pulse->header.configIndex];
                        count = MIN(config->waveform->depth + 50, pulse->header.capacity);
                        *(char *)payload = RKRadarHubTypeScope;
                        payloadSize = sizeof(uint8_t) + 4 * count * sizeof(int16_t);
                        xh = RKGetInt16CDataFromPulse(pulse, 0);
                        xv = RKGetInt16CDataFromPulse(pulse, 1);
                        y = (int16_t *)(payload + 1);
                        RKWaveform *waveform = config->waveform;
                        float scale = 2500.0f * sqrtf((float)waveform->depth);
                        int gid = pulse->header.i % waveform->count;
                        if (gid < 0 || gid >= waveform->count) {
                            RKLog("%s Unexpected gid = %d with filter count %d\n",
                                engine->name, gid, waveform->count);
                            gid = 0;
                        }
                        RKComplex *c = waveform->samples[gid];
                        for (k = 0; k < count - 1; k++) {
                            *(y            ) = xh->i;
                            *(y + count    ) = xh->q;
                            if (k < waveform->depth && engine->radar->developerMode) {
                                *(y + count * 2) = (int16_t)(scale * c[k].i);
                                *(y + count * 3) = (int16_t)(scale * c[k].q);
                            } else {
                                *(y + count * 2) = xv->i;
                                *(y + count * 3) = xv->q;
                            }
                            xh++;
                            xv++;
                            y++;
                        }
                        *(y            ) = -1;
                        *(y + count    ) = 1;
                        *(y + count * 2) = -1;
                        *(y + count * 3) = 1;

                        if (engine->verbose > 1) {
                            RKRadarHubPayloadString(message, payload, payloadSize);
                            RKLog("%s P%04d %s (%zu)\n", engine->name, p, message, payloadSize);
                        }
                        RKWebSocketSend(engine->ws, payload, payloadSize);
                        cummulativePayloadSize += payloadSize;
                        t1 = t0;
                    }
                    // The following line causes a raise condition. Avoid mutex for pulses
                    // pulse->header.s |= RKPulseStatusStreamed;
                    p = RKNextModuloS(p, radar->desc.pulseBufferDepth);
                }
            }

            // payload = engine->payload[2];

            // payload = engine->payload[3];

            // Product Display Data
            #pragma region Product Display
            payload = engine->payload[4 + d];
            if (engine->streams & RKStreamDisplayAll) {
                if (rayConfigIndex != radar->configIndex || (engine->streamsInProgress & RKStreamDisplayAll) != (engine->streams & RKStreamDisplayAll)) {
                    rayConfigIndex = radar->configIndex;
                    engine->streamsInProgress |= (engine->streams & RKStreamDisplayAll);
                    r = radar->rayIndex;
                    if (engine->verbose) {
                        RKLog("%s RKStreamDisplay started.   r = %d\n", engine->name, r);
                    }
                }

                ray = RKGetRayFromBuffer(radar->rays, r);

                if (ray->header.s & RKRayStatusReady) {
                    if (r % engine->rayStride) {
                        usleep(1);
                    } else {
                        // Put together the ray for RadarHub
                        display = (RKRadarHubRay *)payload;

                        stride = (int)ceilf((float)ray->header.gateCount / 512.0f);

                        // T, C, Es, Ee, As, Ae, N, Z0, Z1, Z2, ...
                        display->header.type = RKRadarHubTypeRadialZ;
                        display->header.counter = (uint8_t)((ray->header.i & 0x3F) | ((ray->header.configIndex & 0x03) << 6));
                        display->header.startElevation = (int16_t)roundf(ray->header.startElevation * 32768.0f / 180.0f);
                        display->header.endElevation = (int16_t)roundf(ray->header.endElevation * 32768.0f / 180.0f);
                        display->header.startAzimuth = (uint16_t)(ray->header.startAzimuth * 32768.0f / 180.0f);
                        display->header.endAzimuth = (uint16_t)(ray->header.endAzimuth * 32768.0f / 180.0f);
                        display->header.rangeStart = 0;
                        display->header.rangeDelta = (uint16_t)(ray->header.gateSizeMeters * stride * 10);
                        display->header.gateCount = ray->header.gateCount / stride;
                        payloadSize = sizeof(RKRadarHubRayHeader) + display->header.gateCount * sizeof(RKByte);
                        z = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
                        for (j = 0, k = 0; j < display->header.gateCount; j++, k += stride) {
                            display->data[j] = z[k];
                        }
                        if (j > 512) {
                            RKLog("%s Unexpected display gate count: %d\n", engine->name, j);
                        }
                        if (engine->verbose > 1) {
                            RKRadarHubPayloadString(message, payload, payloadSize);
                            RKLog("%s R%04d %s (%zu)\n", engine->name, r, message, payloadSize);
                        }
                        RKWebSocketSend(engine->ws, payload, payloadSize);
                        cummulativePayloadSize += payloadSize;
                    }
                    ray->header.s |= RKRayStatusStreamed;
                    r = RKNextModuloS(r, radar->desc.rayBufferDepth);
                    d = RKNextModuloS(d, PAYLOAD_DEPTH);
                }
            }

            // Report streaming rate periodically
            gettimeofday(&tv, NULL);
            n0 = (double)tv.tv_sec + 1.0e-6 *(double)tv.tv_usec;
            if ((n0 - n1) > 60.0) {
                rate = cummulativePayloadSize / 1024.0 / (n0 - n1);
                RKLog("%s Streaming at %.1f KB/s\n", engine->name, rate);
                cummulativePayloadSize = 0;
                n1 = n0;
            }

        } else {

            disconnectStreams(engine);

        }
        usleep(1000);
    }

    disconnectStreams(engine);

    if (engine->state & RKEngineStateActive) {
        engine->state ^= RKEngineStateActive;
    }

    return NULL;
}

#pragma region Helper Functions

//
// There are two types of control:
//
// Single push button - {"Label": "Button Label", "Command: "_your_command_"}
// Tandem push buttons - {"Label": "Button Label", "Left: "_your_command_", "Right: "_your_command_"}
//
// where _your_command_ is returned back here to handleMessage() when the button was
// pressed on the GUI
//

#pragma region Delegate Workers

void handleOpen(RKWebSocket *W) {
    RKReporter *engine = (RKReporter *)W->parent;
    RKRadar *radar = engine->radar;
    int r;

    r = snprintf(engine->welcome, sizeof(engine->welcome),
        "%c{"
            "\"command\":\"radarGreet\", "
            "\"pathway\":\"%s\", "
            "\"name\":\"%s\""
        "}",
        RKRadarHubTypeHandshake, engine->pathway, radar->desc.name);
    if (r < 0) {
        RKLog("%s Error. Unable to construct handshake message.\n", engine->name);
    }
    RKLog("%s Sending open packet %s ...\n", engine->name, engine->address);
    if (engine->verbose > 1) {
        RKLog(">%s %s\n", engine->name, engine->welcome);
    }
    RKWebSocketSend(W, engine->welcome, r);
}

void handleClose(RKWebSocket *W) {
    RKReporter *engine = (RKReporter *)W->parent;
    if (engine->state & RKEngineStateActive) {
        engine->state ^= RKEngineStateActive;
    }
    if (engine->verbose > 1) {
        RKLog("%s RKReporter.handleClose()\n", engine->name);
    }
}

void handleMessage(RKWebSocket *W, void *payload, size_t size) {
    RKReporter *engine = (RKReporter *)W->parent;
    RKRadar *radar = engine->radar;

    char *message = (char *)payload;

    if (strstr(message, "Welcome")) {
        engine->connected = true;
        RKLog("%s radar = %s   %s%s%s\n", engine->name,
              radar->desc.name,
              rkGlobalParameters.showColor ? RKMonokaiGreen : "",
              message,
              rkGlobalParameters.showColor ? RKNoColor : "");
        return;
    } else if (strstr(message, "Bye")) {
        engine->connected = false;
        RKLog("%s radar = %s   %s%s%s\n", engine->name,
              radar->desc.name,
              rkGlobalParameters.showColor ? RKMonokaiOrange : "",
              message,
              rkGlobalParameters.showColor ? RKNoColor : "");
        return;
    } else if (message[0] == 's') {
        // Stream
        engine->streams = RKStreamNone;
        if (strstr(message + 1, "h")) {
            engine->streams |= RKStreamHealthInJSON;
        }
        if (strstr(message + 1, "p")) {
            engine->streams |= RKStreamScopeStuff;
        }
        if (strstr(message + 1, "z")) {
            engine->streams |= RKStreamDisplayZ;
        }
        if (strstr(message + 1, "v")) {
            engine->streams |= RKStreamDisplayV;
        }
        if (strstr(message + 1, "w")) {
            engine->streams |= RKStreamDisplayW;
        }
        if (strstr(message + 1, "d")) {
            engine->streams |= RKStreamDisplayD;
        }
        if (strstr(message + 1, "p")) {
            engine->streams |= RKStreamDisplayP;
        }
        if (strstr(message + 1, "r")) {
            engine->streams |= RKStreamDisplayR;
        }
        if (strstr(message + 1, "a")) {
            engine->streams = RKStreamHealthInJSON | RKStreamScopeStuff | RKStreamDisplayZ;
        }
        if (strstr(engine->host, "local")) {
            RKLog("%s Overidding streams for local network\n", engine->name);
            engine->streams = RKStreamHealthInJSON | RKStreamScopeStuff | RKStreamDisplayZ;
        }
        int r = snprintf(engine->message, sizeof(engine->message), "%cACK. Streams -> 0x%08lX", RKRadarHubTypeResponse, (unsigned long)engine->streams);
        RKLog("%s %s%s%s\n", engine->name, rkGlobalParameters.showColor ? RKMonokaiGreen : "", engine->message, rkGlobalParameters.showColor ? RKNoColor : "");
        RKWebSocketSend(W, engine->message, r);
        return;
    }

    if (engine->verbose) {
        RKLog("%s Command '%s%s%s'\n", engine->name,
            rkGlobalParameters.showColor ? RKMonokaiYellow : "",
            message,
            rkGlobalParameters.showColor ? RKNoColor : "");
    }

    char *c;
    char *commandString = message;
    char *commandStringEnd = NULL;

    size_t origin = 0;

    while (commandString) {
        commandStringEnd = strchr(commandString, ';');
        if (commandStringEnd) {
            *commandStringEnd = '\0';
        }
        RKLog("%s Executing '%s%s%s'\n", engine->name,
            rkGlobalParameters.showColor ? RKMonokaiYellow : "",
            commandString,
            rkGlobalParameters.showColor ? RKNoColor : "");

        RKExecuteCommand(radar, commandString, engine->scratch + origin);

        if (strlen(engine->scratch + origin) == 0) {
            RKLog("%s Warning. No response for command '%s%s%s'\n", engine->name,
                rkGlobalParameters.showColor ? RKMonokaiYellow : "",
                commandString,
                rkGlobalParameters.showColor ? RKNoColor : "");
            snprintf(engine->scratch + origin, sizeof(engine->scratch) - origin, "NAK. No response for command '%s'", commandString);
        } else {
            // Only keep the first line of the response. The rest is for terminal display.
            c = strstr(engine->scratch + origin, "\n");
            while (c && c > engine->scratch + origin && (*c == '\n' || *c == '\r')) {
                *c-- = '\0';
            }
            if (*c == '.' || *c == ' ' || *c == '\t') {
                *c++ = ';';
                *c++ = ' ';
                *c = '\0';
            }
        }

        origin += strlen(engine->scratch + origin);

        if (commandStringEnd) {
            commandString = commandStringEnd + 1;
            while (*commandString == ' ' || *commandString == '\t') {
                commandString++;
            }
        } else {
            commandString = NULL;
        }
    }
    // TODO: Need to consolidate the responses. For now, only return the last one.
    int r = snprintf(engine->message, sizeof(engine->message), "%c%s", RKRadarHubTypeResponse, engine->scratch);
    RKStripTail(engine->scratch);
    char b = engine->scratch[0];
    RKLog("%s Response '%s%s%s'\n", engine->name,
        rkGlobalParameters.showColor ? (b == 'A' ? RKMonokaiYellow : RKMonokaiOrange) : "",
        engine->scratch,
        rkGlobalParameters.showColor ? RKNoColor : "");

    RKWebSocketSend(W, engine->message, r);

}

#pragma region Life Cycle

RKReporter *RKReporterInitWithRadarAndHostPathway(RKRadar *radar, const char *host) {
    RKReporter *engine = (RKReporter *)malloc(sizeof(RKReporter));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate an RKReporter.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKReporter));
    engine->radar = radar;
    snprintf(engine->name, sizeof(engine->name), "%s<RadarHubConnect>%s",
             rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorRadarHubReporter) : "",
             rkGlobalParameters.showColor ? RKNoColor : "");
    engine->healthStride = 2;
    engine->pulseStride = 5;
    engine->rayStride = 1;
    engine->memoryUsage = sizeof(RKReporter);
    if (host == NULL || strlen(host) == 0) {
        strcpy(engine->host, "http://localhost:8000");
        strcpy(engine->pathway, "radarkit");
    } else {
        char *c = strchr(host, ' ');
        if (c) {
            *c = '\0';
            char *pathway = c + 1;
            strncpy(engine->pathway, pathway, sizeof(engine->pathway) - 1);
            RKStringLower(engine->pathway);
        } else {
            strcpy(engine->pathway, "radarkit");
        }
        strncpy(engine->host, host, sizeof(engine->host) - 1);
    }
    snprintf(engine->address, sizeof(engine->address), "/ws/radar/%s/", engine->pathway);
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("%s Setting up '%s' (%s%s%s%s%s)\n",
            engine->name, radar->desc.name,
            rkGlobalParameters.showColor ? RKMonokaiGreen : "",
            engine->host, engine->address,
            rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Default streams
    engine->streams = RKStreamHealthInJSON;
    engine->ws = RKWebSocketInit(engine->host, engine->address);
    if (engine->ws == NULL) {
        RKLog("%s Error. Failed to initialize RKWebSocket.\n", engine->name);
        free(engine);
        return NULL;
    }
    RKWebSocketSetOpenHandler(engine->ws, &handleOpen);
    RKWebSocketSetCloseHandler(engine->ws, &handleClose);
    RKWebSocketSetMessageHandler(engine->ws, &handleMessage);
    RKWebSocketSetVerbose(engine->ws, engine->verbose > 1 ? 2 : 0);
    RKWebSocketSetParent(engine->ws, engine);
    return engine;
}

RKReporter *RKReporterInitForRadarHub(RKRadar *radar) {
    return RKReporterInitWithRadarAndHostPathway(radar, "https://radarhub.arrc.ou.edu");
}

RKReporter *RKReporterInitForLocal(RKRadar *radar) {
    return RKReporterInitWithRadarAndHostPathway(radar, "localhost:8000 radarkit");
}

RKReporter *RKReporterInit(void) {
    return RKReporterInitForLocal(NULL);
}

void RKReporterFree(RKReporter *engine) {
    if (engine->ws) {
        RKWebSocketFree(engine->ws);
    }
    free(engine);
}

#pragma region Properties

void RKReporterSetVerbose(RKReporter *engine, const int verbose) {
    engine->verbose = verbose;
    if (engine->ws) {
        RKWebSocketSetVerbose(engine->ws, verbose - 1);
    }
}

#pragma region Interactions

void RKReporterStart(RKReporter *engine) {
    if (engine->ws == NULL) {
        RKLog("%s Error. RKWebSocket is not initialized.\n", engine->name);
        return;
    }
    RKLog("%s Starting ...\n", engine->name);
    if (!strcmp(engine->host, "nohost")) {
        RKLog(">%s Not running until it is official\n", engine->name);
        return;
    }
    RKWebSocketStart(engine->ws);
    engine->state |= RKEngineStateWantActive;
    if (pthread_create(&engine->ticWorker, NULL, reporter, engine)) {
        RKLog("%s Error. Failed to create reporter().\n", engine->name);
        return;
    }
    do {
        usleep(10000);
    } while (engine->state & RKEngineStateWantActive && !(engine->state & RKEngineStateActive));
}

void RKReporterStop(RKReporter *engine) {
    RKLog("%s Stopping ...\n", engine->name);
    if (engine->state & RKEngineStateWantActive) {
        engine->state ^= RKEngineStateWantActive;
    }
    int s = 0;
    do {
        usleep(10000);
    } while (engine->state & RKEngineStateActive && s++ < 300);
    if (engine->ws->wantActive) {
        RKLog("%s Disconnecting %s:%s ...\n", engine->name, engine->ws->host, engine->ws->path);
        RKWebSocketStop(engine->ws);
    }
    RKLog("%s Stopped.\n", engine->name);
}
