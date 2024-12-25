//
//  RKReporter.h
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2022 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Reporter__
#define __RadarKit_Reporter__

#include <RadarKit/RKRadar.h>

// Keep PAYLOAD_DEPTH of display payload. 0 - Health, 1 - Scope, 2 - Reserved, 3 - Reserved, 4+ - Display
#define PAYLOAD_DEPTH       8
#define PAYLOAD_CAPACITY    (1024 * 1024)

typedef struct rk_reporter {
    // User set variables
    RKName                           name;
    RKName                           host;
    char                             pathway[RKNameLength];
    char                             address[RKNameLength + 16];
    char                             welcome[RKMaximumStringLength];
    char                             control[RKMaximumStringLength];
    char                             message[RKMaximumStringLength];
    char                             scratch[RKMaximumStringLength];
    char                             payload[4 + PAYLOAD_DEPTH][PAYLOAD_CAPACITY];
    int                              verbose;
    RKRadar                          *radar;

    // Program set variables
    RKWebSocket                      *ws;
    pthread_t                        ticWorker;
    pthread_mutex_t                  mutex;

    // Status / health
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    bool                             connected;
    size_t                           memoryUsage;

    RKStream                         streams;
    RKStream                         streamsInProgress;
    uint32_t                         healthStride;
    uint32_t                         pulseStride;
    uint32_t                         rayStride;
    uint32_t                         fps;
    char                             string[RKMaximumPacketSize];

} RKReporter;

RKReporter *RKReporterInitWithHost(const char *);
RKReporter *RKReporterInitForRadarHub(void);
RKReporter *RKReporterInitForLocal(void);
RKReporter *RKReporterInit(void);
void RKReporterFree(RKReporter *);

void RKReporterSetRadar(RKReporter *, RKRadar *);
void RKReporterSetVerbose(RKReporter *engine, const int verbose);

void RKReporterStart(RKReporter *);
void RKReporterStop(RKReporter *);

#endif
