//
//  RKReporter.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2023 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKReporter.h>

// Private declarations

#pragma mark - Busy Loop

void *reporter(void *in) {
    RKReporter *engine = (RKReporter *)in;
    RKRadar *radar = engine->radar;

    int s = 0;
    int k = 0;

    RKLog("%s Started.  mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));

    engine->state |= RKEngineStateActive;

    while (engine->state & RKEngineStateActive) {
        RKLog("%s Busy %d\n", engine->name, k);
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
    RKReporter *R = (RKReporter *)w->parent;
    if (R->verbose) {
        printf("RKReporter: ONOPEN\n");
    }
    int r;
    r = sprintf(R->welcome,
        "%c{"
            "\"name\":\"%s\", "
            "\"command\":\"radarConnect\""
        "}",
        RadarHubTypeHandshake, R->radar->desc.name);
    RKLog("%s Sending open packet ...\n", R->name);
    printf("%s\n", R->welcome);
    RKWebSocketSend(w, R->welcome, r);
//    sendControl(w);
}

void handleClose(RKWebSocket *W) {
//    R->connected = false;
//    if (R->verbose) {
//        printf("ONCLOSE\n");
//    }
    RKReporter *reporter = (RKReporter *)W->parent;
    if (reporter->state & RKEngineStateActive) {
        reporter->state ^= RKEngineStateActive;
    }
    RKLog("%s WebSocket handleClose()\n", reporter->name);
}

void handleMessage(RKWebSocket *W, void *payload, size_t size) {
    RKReporter *reporter = (RKReporter *)W->parent;
    RKRadar *radar = reporter->radar;
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
    RKName node;
    strcpy(node, radar->desc.name);
    RKStringLower(node);
    sprintf(engine->address, "/ws/radar/%s/", node);
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
