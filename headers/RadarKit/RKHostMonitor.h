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

//typedef char RKHostAddress[RKNameLength];

struct rk_unit_monitor {
    int                    id;
    uint64_t               tic;
    pthread_t              tid;
    RKHostMonitor          *parent;
    uint16_t               pingIntervalInSeconds;
    uint16_t               sequenceNumber;
    uint16_t               identifier;
    struct timeval         latestTime;
    RKHostStatus           hostStatus;
};

struct rk_host_monitor {
    // User set variables
    RKName                 name;
    uint8_t                verbose;                             // Verbosity level
    RKName                 *hosts;                              // List of names (hostnames)
    
    // Program set variables
    uint64_t               tic;
    int                    workerCount;
    RKUnitMonitor          *workers;
    pthread_t              tidHostWatcher;
    pthread_mutex_t        mutex;
    bool                   allKnown;
    bool                   allReachable;
    bool                   anyReachable;

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
