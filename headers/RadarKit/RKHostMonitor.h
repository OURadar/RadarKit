//
//  RKHostMonitor.h
//  RadarKit
//
//  Created by Boonleng Cheong on 2/24/18.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_HostMonitor__
#define __RadarKit_HostMonitor__

#include <RadarKit/RKFoundation.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct rk_host_target RKHostTarget;
typedef struct rk_host_monitor RKHostMonitor;
struct rk_host_target {
    RKName                           name;
    int                              port;
    bool                             known;
    bool                             reachable;
};

struct rk_host_monitor {
    // User set variables
    RKName                           name;
    uint8_t                          verbose;                                  // Verbosity level
    RKHostTarget                     *hosts;                                   // List of hosts

    // Program set variables
    uint64_t                         tic;
    uint8_t                          hostCount;
    pthread_t                        tidHostWatcher;
    bool                             allKnown;
    bool                             allReachable;
    bool                             anyReachable;

    // Status / health
    RKEngineState                    state;
    uint32_t                         memoryUsage;
};

RKHostMonitor *RKHostMonitorInit(void);
void RKHostMonitorFree(RKHostMonitor *);

void RKHostMonitorSetVerbose(RKHostMonitor *, const int);
void RKHostMonitorAddHost(RKHostMonitor *, const char *);

int RKHostMonitorStart(RKHostMonitor *);
int RKHostMonitorStop(RKHostMonitor *);

#endif
