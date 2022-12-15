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

typedef struct rk_reporter {
    // User set variables
    RKName                           name;
    RKName                           host;
    RKName                           address;
    char                             welcome[RKMaximumStringLength];
    char                             control[RKMaximumStringLength];
    char                             message[RKMaximumStringLength];
    int                              verbose;
    RKRadar                          *radar;

    // Program set variables
    RKWebSocket                      *ws;
    RKWebSocketSSLFlag               flag;
    pthread_t                        tidReportWorker;
    pthread_mutex_t                  mutex;

    // Status / health
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    size_t                           memoryUsage;
} RKReporter;

RKReporter *RKReporterInitWithHost(const char *);
RKReporter *RKReporterInit(void);
void RKReporterFree(RKReporter *);

void RKReporterSetRadar(RKReporter *, RKRadar *);

void RKReporterStart(RKReporter *);
void RKReporterStop(RKReporter *);

#endif
