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
    strncpy(engine->host, host, RKNameLength);
    return engine;
}

RKReporter *RKReporterInit(void) {
    return RKReporterInitWithHost("http://localhost:8001");
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
    sprintf(engine->address, "/ws/radar/%s/", radar->name);
    RKLog("Setting up radar $s ... %s\n", radar->name, engine->address);
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

}

void RKReporterStop(RKReporter *engine) {

}
