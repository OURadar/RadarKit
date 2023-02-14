//
//  RKReporter.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2022 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKReporter.h>

// Private declarations

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

void handleOpen(RKWebSocket *w) {
//    if (R->verbose) {
//        printf("ONOPEN\n");
//    }
//    int r;
//    r = sprintf(R->welcome,
//        "%c{"
//            "\"radar\":\"%s\", "
//            "\"command\":\"radarConnect\""
//        "}",
//        RadarHubTypeHandshake, R->name);
//    RKWebSocketSend(R->ws, R->welcome, r);
//    sendControl(w);
}

void handleClose(RKWebSocket *W) {
//    R->connected = false;
//    if (R->verbose) {
//        printf("ONCLOSE\n");
//    }
}

void handleMessage(RKWebSocket *W, void *payload, size_t size) {
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
    // char *c = strstr(host, "://");
    // if (c) {
    //     strncpy(engine->host, c + 3, RKNameLength);
    // } else if (host) {
    //     strncpy(engine->host, host, RKNameLength);
    // }
    strncpy(engine->host, host, RKNameLength);
    RKLog("%s host = %s\n", engine->name, engine->host);
    return engine;
}

RKReporter *RKReporterInitForRadarHub(void) {
    return RKReporterInitWithHost("https://radarhub.arrc.ou.edu");
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
}

void RKReporterSetRadar(RKReporter *engine, RKRadar *radar) {
    engine->radar = radar;
    RKName name;
    strcpy(name, radar->desc.name);
    RKStringLower(name);
    sprintf(engine->address, "/ws/radar/%s/", name);
    RKLog("%s Setting up radar %s ... %s\n", engine->name, name, engine->address);
    if (strstr(engine->host, "https") != NULL) {
        engine->flag = RKWebSocketFlagSSLOn;
    } else {
        engine->flag = RKWebSocketFlagSSLOff;
    }
    engine->ws = RKWebSocketInit(engine->host, engine->address, engine->flag);
    RKWebSocketSetOpenHandler(engine->ws, &handleOpen);
    RKWebSocketSetCloseHandler(engine->ws, &handleClose);
    RKWebSocketSetMessageHandler(engine->ws, &handleMessage);
}

#pragma mark - Interactions

void RKReporterStart(RKReporter *engine) {
    RKLog("%s Starting ...\n", engine->name);
}

void RKReporterStop(RKReporter *engine) {
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    RKWebSocketStop(engine->ws);
    RKLog("%s Stopped.\n", engine->name);
}
