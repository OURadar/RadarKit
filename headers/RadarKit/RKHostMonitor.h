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

typedef struct rk_unit_monitor RKUnitMonitor;
typedef struct rk_host_monitor RKHostMonitor;

typedef char RKHostAddress[RKNameLength];

struct rk_unit_monitor {
    int                    id;
    int                    tic;
    pthread_t              tid;
    RKHostMonitor          *parent;
    uint16_t               sequenceNumber;
    uint16_t               identifier;
    RKHostState            state;
};

struct rk_host_monitor {
    // User set variables
    char                   name[RKNameLength];
    uint8_t                verbose;                             // Verbosity level
    RKHostAddress          *hosts;
    
    // Program set variables
    int                    tic;
    int                    workerCount;
    RKUnitMonitor          *workers;
    pthread_t              tidHostWatcher;
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
