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

#ifndef __RadarHub_Types__
#define __RadarHub_Types__

enum RadarHubType {
    RadarHubTypeHandshake       = 1,             // JSON message {"radar":"px1000","command":"radarConnect"}
    RadarHubTypeControl         = 2,             // JSON control {"Go":{...},"Stop":{...},...}
    RadarHubTypeHealth          = 3,             // JSON health {"Transceiver":{...},"Pedestal":{...},...}
    RadarHubTypeReserve4        = 4,             //
    RadarHubTypeScope           = 5,             // Scope data in binary
    RadarHubTypeResponse        = 6,             // Plain text response
    RadarHubTypeReserved7       = 7,             //
    RadarHubTypeReserved8       = 8,             //
    RadarHubTypeReserved9       = 9,             //
    RadarHubTypeReserved10      = 10,            //
    RadarHubTypeReserved11      = 11,            //
    RadarHubTypeReserved12      = 12,            //
    RadarHubTypeReserved13      = 13,            //
    RadarHubTypeReserved14      = 14,            //
    RadarHubTypeReserved15      = 15,            //
    RadarHubTypeRadialZ         = 16,            //
    RadarHubTypeRadialV         = 17,            //
    RadarHubTypeRadialW         = 18,            //
    RadarHubTypeRadialD         = 19,            //
    RadarHubTypeRadialP         = 20,            //
    RadarHubTypeRadialR         = 21             //
};

enum Blah {
    BlahOne,
    BlahTwo,
    BlahThree
};

#endif

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
RKReporter *RKReporterInitForRadarHub(void);
RKReporter *RKReporterInitForLocal(void);
RKReporter *RKReporterInit(void);
void RKReporterFree(RKReporter *);

void RKReporterSetRadar(RKReporter *, RKRadar *);
void RKReporterSetVerbose(RKReporter *engine, const int verbose);

void RKReporterStart(RKReporter *);
void RKReporterStop(RKReporter *);

#endif
