//
//  RKReporter.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKReporter.h>

// Private declarations

#define PAYLOAD_CAPACITY    (1024 * 1024)

#pragma mark - Busy Loop

void *reporter(void *in) {
    RKReporter *engine = (RKReporter *)in;
    RKRadar *radar = engine->radar;

    int j;
    int k = 0;

    int s = 0;
    char *c;
    uint32_t index;

    void *payload = malloc(PAYLOAD_CAPACITY);
    size_t payload_size;

    RKLog("%s Started.  mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));

    engine->state |= RKEngineStateActive;

    while (engine->state & RKEngineStateActive) {
        RKLog("%s Busy %d\n", engine->name, k);
        
        // Health Status
        #pragma mark Health Status
        
        if (engine->streams & RKStreamHealthInJSON) {
            index = radar->healthIndex;

            if (!(engine->streamsInProgress & RKStreamHealthInJSON)) {
                engine->streamsInProgress |= RKStreamHealthInJSON;
                if (engine->verbose) {
                    RKLog("%s Begin streaming RKHealth -> %d (0x%x %s).\n", engine->name, index,
                          radar->healths[index].flag,
                          radar->healths[index].flag == RKStatusFlagVacant ? "vacant" : "ready");
                }
                engine->healthIndex = index;
            }

            engine->state |= RKEngineStateSleep1;
            s = 0;
            while (radar->healths[engine->healthIndex].flag != RKHealthFlagReady && engine->ws->connected && s++ < 20) {
                if (s % 10 == 0 && engine->verbose > 1) {
                    RKLog("%s sleep 0/%.1f s  RKHealth\n", engine->name, s * 0.1f);
                }
                usleep(50000);
            }
            engine->state ^= RKEngineStateSleep1;

            if (radar->healths[engine->healthIndex].flag == RKHealthFlagReady && engine->ws->connected) {
                while (engine->healthIndex != index) {
                    snprintf(payload, PAYLOAD_CAPACITY, "%c%s", RKRadarHubTypeHealth, radar->healths[engine->healthIndex].string);
                    payload_size = 1 + strlen((char *)(payload + 1));
                    c = payload + payload_size - 2;
                    RKLog("%s Sending health packet s=%zu  MSG = ... %c(%d) %c(%d) (%d)\n",
                          engine->name, payload_size, *c, *c, *(c + 1), *(c + 1), (int)*(c + 2));
                    RKWebSocketSend(engine->ws, payload, payload_size);
                    engine->healthIndex = RKNextModuloS(engine->healthIndex, radar->desc.healthBufferDepth);
                }
            } else {
                RKLog("%s No Health / Deactivated.   healthIndex = %d / %d\n", engine->name, engine->healthIndex, radar->healthIndex);
            }

        }
        
        s = 0;
        do {
            usleep(100000);
        } while (engine->state & RKEngineStateActive && s++ < 10);
    }

    return NULL;
}

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

void handleOpen(RKWebSocket *w) {
    RKReporter *engine = (RKReporter *)w->parent;
    if (engine->verbose) {
        printf("RKReporter: ONOPEN\n");
    }
    size_t r = sprintf(engine->welcome,
        "%c{"
            "\"command\":\"radarConnect\", "
            "\"pathway\":\"%s\", "
            "\"name\":\"%s\""
        "}",
        RadarHubTypeHandshake, engine->pathway, engine->radar->desc.name);
    RKLog("%s Sending open packet %s ...\n", engine->name, engine->address);
    RKLog("%s %s\n", engine->name, engine->welcome);
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
    RKLog("%s\n", engine->name);
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
    engine->streams = RKStreamHealthInJSON;
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
