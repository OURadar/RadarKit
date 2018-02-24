//
//  RKHostMonitor.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_HostMonitor__
#define __RadarKit_HostMonitor__

#include <RadarKit/RKFoundation.h>

typedef struct rk_unit_checker RKUnitChecker;
typedef struct rk_host_monitor RKHostMonitor;

typedef char RKUnitHost[RKNameLength];

struct rk_unit_checker {
    int                    tic;
    pthread_t              tid;
    char                   address[RKNameLength];
    RKHostMonitor          *parent;
};

struct rk_host_monitor {
    // User set variables
    char                   name[RKNameLength];
    uint8_t                verbose;                             // Verbosity level
    RKUnitHost             *hosts;
    
    // Program set variables
    int                    workerCount;
    RKUnitChecker          *workers;
    pthread_mutex_t        mutex;

    // Status / health
    RKEngineState          state;
    uint32_t               memoryUsage;
};

RKHostMonitor *RKHostMonitorInit(void);
void RKHostMonitorFree(RKHostMonitor *);

void RKHostMonitorSetVerbose(RKHostMonitor *, const int);
void RKHostMonitorAddHost(RKHostMonitor *, const char *);

int RKHostMonitorStart(RKHostMonitor *);
int RKHostMonitorStop(RKHostMonitor *);

#endif
