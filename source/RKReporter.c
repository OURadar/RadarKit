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

    int d = 0;
    int h = 0;
    int p = 0;
    int r = 0;

    size_t payload_size;
    int count;
    int k;

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

    do {
        usleep(10000);
    } while (engine->ws->tic < 1);

    RKLog("%s Started.  mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));

    engine->state |= RKEngineStateActive;

    while (engine->state & RKEngineStateWantActive) {
        if (engine->ws->connected) {

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
                        payload_size = 1 + strlen((char *)(payload + 1));
                        if (engine->verbose > 1) {
                            if (payload_size < 64) {
                                RKBinaryString(message, payload, payload_size);
                            } else {
                                RKHeadTailBinaryString(message, payload, payload_size);
                            }
                            RKLog("%s H%04d %s (%zu)", engine->name, h, message, payload_size);
                        }
                        RKWebSocketSend(engine->ws, payload, payload_size);
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

                if (pulse->header.s & RKPulseStatusHasIQData) {
                    if (p % engine->pulseStride) {
                        usleep(1);
                    } else {
                        // Use payload 1 as target
                        payload = engine->payload[1];
                        // Put together the payload for RadarHub
                        //count = MIN(200, pulse->header.gateCount);
                        config = &radar->configs[pulse->header.configIndex];
                        count = config->waveform->depth + 50;
                        *(char *)payload = RKRadarHubTypeScope;
                        payload_size = sizeof(uint8_t) + 4 * count * sizeof(int16_t);
                        xh = RKGetInt16CDataFromPulse(pulse, 0);
                        xv = RKGetInt16CDataFromPulse(pulse, 1);
                        y = (int16_t *)(payload + 1);
                        for (k = 0; k < count - 1; k++) {
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
                            RKRadarHubPayloadString(message, payload, payload_size);
                            RKLog("%s P%04d %s (%zu)\n", engine->name, p, message, payload_size);
                        }
                        RKWebSocketSend(engine->ws, payload, payload_size);
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
                if ((engine->streamsInProgress & RKStreamDisplayAll) != (engine->streams & RKStreamDisplayAll)) {
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
                        // T, C, Es, Ee, As, Ae, N, Z0, Z1, Z2, ...
                        display->header.type = RKRadarHubTypeRadialZ;
                        display->header.counter = (uint8_t)((ray->header.i & 0x3F) | ((ray->header.configIndex & 0x03) << 6));
                        display->header.startElevation = (int16_t)roundf(ray->header.startElevation * 32768.0f / 180.0f);
                        display->header.endElevation = (int16_t)roundf(ray->header.endElevation * 32768.0f / 180.0f);
                        display->header.startAzimuth = (uint16_t)(ray->header.startAzimuth * 32768.0f / 180.0f);
                        display->header.endAzimuth = (uint16_t)(ray->header.endAzimuth * 32768.0f / 180.0f);
                        display->header.rangeStart = 0;
                        display->header.rangeDelta = ray->header.gateSizeMeters * 10;
                        display->header.gateCount = MIN(512, ray->header.gateCount);
                        payload_size = sizeof(RKRadarHubRayHeader) + display->header.gateCount * sizeof(RKByte);
                        z = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
                        memcpy(display->data, z, display->header.gateCount * sizeof(RKByte));
                        if (engine->verbose > 1) {
                            RKRadarHubPayloadString(message, payload, payload_size);
                            RKLog("%s R%04d %s (%zu)\n", engine->name, r, message, payload_size);
                        }
                        RKWebSocketSend(engine->ws, payload, payload_size);
                    }
                    ray->header.s |= RKRayStatusStreamed;
                    r = RKNextModuloS(r, radar->desc.rayBufferDepth);
                    d = RKNextModuloS(d, PAYLOAD_DEPTH);
                }
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

#pragma mark - Helper Functions

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

    r = sprintf(engine->welcome,
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
    //
    r = sprintf(engine->control,
        "%c{"
            "\"pathway\": \"%s\", "
            "\"control\": ["
                "{\"Label\":\"Go\", \"Command\":\"t y\"}, "
                "{\"Label\":\"Stop\", \"Command\":\"t z\"}, "
                "{\"Label\":\"Stop Pedestal\", \"Command\":\"p stop\"}, "
                "{\"Label\":\"Park\", \"Command\":\"p point 0 90\"}, "
                "{\"Label\":\"%d FPS\", \"Left\":\"d f-\", \"Right\":\"d f+\"}, "
                "{\"Label\":\"PRF 1,000 Hz (84 km)\", \"Command\":\"t prf 1000\"}, "
                "{\"Label\":\"PRF 1,475 Hz (75 km)\", \"Command\":\"t prf 1475\"}, "
                "{\"Label\":\"PRF 2,000 Hz (65 km)\", \"Command\":\"t prf 2000\"}, "
                "{\"Label\":\"Measure Noise\", \"Command\":\"t n\"}, "
                "{\"Label\":\"10us pulse\", \"Command\":\"t w s10\"}, "
                "{\"Label\":\"20us LFM\", \"Command\":\"t w q0420\"}, "
                "{\"Label\":\"50us pulse\", \"Command\":\"t w s50\"}, "
                "{\"Label\":\"TFM + OFM\", \"Command\":\"t w ofm\"}, "
                "{\"Label\":\"OFM\", \"Command\":\"t w ofmd\"}, "
                "{\"Label\":\"1-tilt EL 3.0 deg @ 5 deg/s\", \"Command\":\"p ppi 3 5\"}, "
                "{\"Label\":\"1-tilt EL 5.0 deg @ 20 deg/s\", \"Command\":\"p ppi 5 20\"}"
            "]"
        "}",
        RKRadarHubTypeControl, engine->pathway, engine->fps);
    if (r < 0) {
        RKLog("%s Error. Unable to construct control JSON.\n", engine->name);
    }
    RKWebSocketSend(W, engine->control, strlen(engine->control));
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
    }

    if (engine->verbose) {
        RKLog("%s '%s'\n", engine->name, message);
    }

    int c = rand() % 3;
    int r = sprintf(engine->message, "%c%c%s",
                    RKRadarHubTypeResponse,
                    c == 0 ? 'A' : (c == 1 ? 'Q' : 'N'),
                    message);
    RKLog("%s %s%s%s\n", engine->name,
          rkGlobalParameters.showColor ? (c == 0 ? RKMonokaiGreen :
                                          (c == 1 ? RKBaseGreenColor : RKMonokaiOrange)) : "",
          engine->message,
          rkGlobalParameters.showColor ? RKNoColor : "");

    char mode = message[0];
    char *args = message + 1;
    while (*args == ' ' && *args != '\0') {
        args++;
    }
    switch (mode) {
        case 's':
            RKLog("Switching streams to '%s' ...\n", args);
        default:
            break;
    }
    // Repeat the incoming message with prefix 'A', 'Q', or 'N'
    RKWebSocketSend(W, engine->message, r);
}

#pragma region Life Cycle

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
    engine->pulseStride = 10;
    engine->rayStride = 1;
    engine->verbose = 1;
    engine->memoryUsage = sizeof(RKReporter);
    if (strlen(host) == 0 || host == NULL) {
        // sprintf(engine->host, "http://localhost:8000");
        sprintf(engine->host, "nohost");
    } else {
        strncpy(engine->host, host, sizeof(engine->host) - 1);
    }
    // Default streams
    engine->streams = RKStreamHealthInJSON | RKStreamScopeStuff | RKStreamDisplayZ;
    // engine->streams = RKStreamHealthInJSON;
    return engine;
}

RKReporter *RKReporterInitForRadarHub(void) {
    return RKReporterInitWithHost("https://radarhub.arrc.ou.edu");
}

RKReporter *RKReporterInitForLocal(void) {
    return RKReporterInitWithHost("localhost:8000");
}

RKReporter *RKReporterInit(void) {
    return RKReporterInitForLocal();
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

void RKReporterSetRadar(RKReporter *engine, RKRadar *radar) {
    engine->radar = radar;
    strcpy(engine->pathway, radar->desc.name);
    RKStringLower(engine->pathway);
    snprintf(engine->address, sizeof(engine->address), "/ws/radar/%s/", engine->pathway);
    if (engine->verbose > 1) {
        RKLog("%s Setting up radar %s (%s @ %s)\n", engine->name, radar->desc.name, engine->host, engine->address);
    }
    engine->ws = RKWebSocketInit(engine->host, engine->address);
    RKWebSocketSetOpenHandler(engine->ws, &handleOpen);
    RKWebSocketSetCloseHandler(engine->ws, &handleClose);
    RKWebSocketSetMessageHandler(engine->ws, &handleMessage);
    RKWebSocketSetVerbose(engine->ws, engine->verbose > 1 ? 2 : 0);
    RKWebSocketSetParent(engine->ws, engine);
}

#pragma region Interactions

void RKReporterStart(RKReporter *engine) {
    RKLog("%s Starting ...\n", engine->name);
    if (!strcmp(engine->host, "nohost")) {
        RKLog(">%s Not running until it is official\n", engine->name);
        return;
    }
    RKLog(">%s host = %s%s:%d%s%s\n", engine->name,
          rkGlobalParameters.showColor ? (engine->ws->useSSL ? RKMonokaiGreen : RKMonokaiYellow) : "",
          engine->ws->host, engine->ws->port, engine->ws->path,
          rkGlobalParameters.showColor ? RKNoColor : "");
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
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
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
